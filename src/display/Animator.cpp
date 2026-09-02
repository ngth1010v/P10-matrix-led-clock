#include "Animator.h"
#include <algorithm>

Animator::~Animator() {
    if (workerTaskHandle != NULL) {
        vTaskDelete(workerTaskHandle);
        workerTaskHandle = NULL;
    }
    if (batchQueue != NULL) {
        vQueueDelete(batchQueue);
        batchQueue = NULL;
    }
}

void Animator::init(P10Driver* p10Driver, FontRenderer* pFontRenderer) {
    if (isInitialized) return;

    this->driver = p10Driver;
    this->fontRenderer = pFontRenderer;

    // Queue holds AnimationBatch pointers to conserve stack space
    batchQueue = xQueueCreate(MAX_ANIMATE_BATCH, sizeof(AnimationBatch*));

    if (batchQueue != NULL) {
        xTaskCreatePinnedToCore(
            animationWorkerTask,
            "AnimatorTask",
            4096,
            this,
            1,
            &workerTaskHandle,
            1 // Run on Core 1 alongside primary task processing
        );
        isInitialized = true;
    }
}

void Animator::animate(uint x, uint y, std::string fromText, std::string toText, bool mini, uint delay, bool alignTop, bool alignLeft) {
    AnimationTask task;
    task.x = x;
    task.y = y;
    task.fromText = fromText;
    task.toText = toText;
    task.mini = mini;
    task.delay = delay;
    task.alignTop = alignTop;
    task.alignLeft = alignLeft;

    currentBuildingBatch.push_back(task);
}

void Animator::startAnimate() {
    if (currentBuildingBatch.empty()) return;

    // Allocate batch object on heap to pass through FreeRTOS queue safely
    AnimationBatch* batchPtr = new AnimationBatch();
    batchPtr->tasks = std::move(currentBuildingBatch);
    currentBuildingBatch.clear();

    // Drop new batch if batch queue capacity exceeded (> MAX_ANIMATE_BATCH)
    if (xQueueSend(batchQueue, &batchPtr, 0) != pdPASS) {
        delete batchPtr; // Queue full, drop batch
    }
}

FontRenderer::Bitmap Animator::renderStringToBitmap(const std::string& text, bool mini) {
    FontRenderer::Bitmap compositeBmp;
    compositeBmp.w = 0;
    compositeBmp.h = 0;

    if (text.empty() || !fontRenderer) return compositeBmp;

    std::vector<FontRenderer::Bitmap> charBitmaps;
    charBitmaps.reserve(text.length());

    uint8_t totalW = 0;
    uint8_t maxH = 0;

    for (char c : text) {
        FontRenderer::Bitmap cbmp = fontRenderer->get(c, mini);
        totalW += cbmp.w; // 0-pixel spacing between characters
        if (cbmp.h > maxH) maxH = cbmp.h;
        charBitmaps.push_back(cbmp);
    }

    compositeBmp.w = totalW;
    compositeBmp.h = maxH;
    compositeBmp.pixels.resize(totalW, std::vector<bool>(maxH, false));

    uint8_t currentX = 0;
    for (const auto& cbmp : charBitmaps) {
        for (uint8_t cx = 0; cx < cbmp.w; ++cx) {
            for (uint8_t cy = 0; cy < cbmp.h; ++cy) {
                if (cx < cbmp.pixels.size() && cy < cbmp.pixels[cx].size()) {
                    compositeBmp.pixels[currentX + cx][cy] = cbmp.pixels[cx][cy];
                }
            }
        }
        currentX += cbmp.w;
    }

    return compositeBmp;
}

void Animator::animationWorkerTask(void* pvParameters) {
    Animator* animator = static_cast<Animator*>(pvParameters);
    AnimationBatch* batchPtr = nullptr;

    while (true) {
        if (xQueueReceive(animator->batchQueue, &batchPtr, portMAX_DELAY) == pdTRUE) {
            if (batchPtr) {
                animator->processBatch(*batchPtr);
                delete batchPtr;
            }
        }
    }
}

void Animator::processBatch(const AnimationBatch& batch) {
    if (batch.tasks.empty() || !driver) return;

    struct PreparedTask {
        uint x;
        uint y;
        uint delay;
        FontRenderer::Bitmap fromBmp;
        FontRenderer::Bitmap toBmp;
        uint maxW;
        uint maxH;
        uint fromXOffset;
        uint toXOffset;
        uint fromYOffset;
        uint toYOffset;
    };

    std::vector<PreparedTask> prepared;
    prepared.reserve(batch.tasks.size());

    uint32_t maxTotalTaskTime = 0;

    for (const auto& task : batch.tasks) {
        PreparedTask pt;
        pt.x = task.x;
        pt.y = task.y;
        pt.delay = task.delay;
        pt.fromBmp = renderStringToBitmap(task.fromText, task.mini);
        pt.toBmp = renderStringToBitmap(task.toText, task.mini);

        pt.maxW = std::max(pt.fromBmp.w, pt.toBmp.w);
        pt.maxH = std::max(pt.fromBmp.h, pt.toBmp.h);

        // Horizontal alignment offsets within maxW bounding box
        pt.fromXOffset = task.alignLeft ? 0 : (pt.maxW - pt.fromBmp.w);
        pt.toXOffset   = task.alignLeft ? 0 : (pt.maxW - pt.toBmp.w);

        // Vertical alignment offsets within maxH bounding box
        pt.fromYOffset = task.alignTop ? 0 : (pt.maxH - pt.fromBmp.h);
        pt.toYOffset   = task.alignTop ? 0 : (pt.maxH - pt.toBmp.h);

        uint32_t taskTime = task.delay + ANIMATE_DURATION;
        if (taskTime > maxTotalTaskTime) {
            maxTotalTaskTime = taskTime;
        }

        prepared.push_back(pt);
    }

    uint32_t startTime = millis();
    const uint32_t frameIntervalMs = 20; // 50 FPS refresh loop

    while (true) {
        uint32_t elapsed = millis() - startTime;
        if (elapsed >= maxTotalTaskTime) break;

        for (const auto& pt : prepared) {
            if (pt.maxW == 0 || pt.maxH == 0) continue;

            float progress = 0.0f;
            if (elapsed > pt.delay) {
                uint32_t animElapsed = elapsed - pt.delay;
                progress = (float)animElapsed / (float)ANIMATE_DURATION;
                if (progress > 1.0f) progress = 1.0f;
            }

            float easedProgress = easeInOutSine(progress);

            // Total vertical distance is maxH + 1px gap
            const int totalDistance = (int)pt.maxH + 1;
            int shiftY = (int)std::round(easedProgress * totalDistance);

            for (uint localX = 0; localX < pt.maxW; ++localX) {
                for (uint localY = 0; localY < pt.maxH; ++localY) {
                    bool pixelOn = false;

                    // Exiting bitmap (fromText) sliding down out of crop box
                    int fromSourceX = (int)localX - (int)pt.fromXOffset;
                    int fromSourceY = (int)localY - (int)pt.fromYOffset - shiftY;

                    if (fromSourceX >= 0 && fromSourceX < (int)pt.fromBmp.w &&
                        fromSourceY >= 0 && fromSourceY < (int)pt.fromBmp.h) {
                        pixelOn = pt.fromBmp.pixels[fromSourceX][fromSourceY];
                    }

                    // Entering bitmap (toText) sliding down into crop box from top
                    int toSourceX = (int)localX - (int)pt.toXOffset;
                    int toSourceY = (int)localY - (int)pt.toYOffset - shiftY + totalDistance;

                    if (toSourceX >= 0 && toSourceX < (int)pt.toBmp.w &&
                        toSourceY >= 0 && toSourceY < (int)pt.toBmp.h) {
                        pixelOn = pixelOn || pt.toBmp.pixels[toSourceX][toSourceY];
                    }

                    driver->set(pt.x + localX, pt.y + localY, pixelOn);
                }
            }
        }

        driver->flush();
        vTaskDelay(pdMS_TO_TICKS(frameIntervalMs));
    }

    // Final frame lock to ensure 100% state target representation
    for (const auto& pt : prepared) {
        for (uint localX = 0; localX < pt.maxW; ++localX) {
            for (uint localY = 0; localY < pt.maxH; ++localY) {
                bool pixelOn = false;

                int toSourceX = (int)localX - (int)pt.toXOffset;
                int toSourceY = (int)localY - (int)pt.toYOffset;

                if (toSourceX >= 0 && toSourceX < (int)pt.toBmp.w &&
                    toSourceY >= 0 && toSourceY < (int)pt.toBmp.h) {
                    pixelOn = pt.toBmp.pixels[toSourceX][toSourceY];
                }

                driver->set(pt.x + localX, pt.y + localY, pixelOn);
            }
        }
    }
    driver->flush();
}