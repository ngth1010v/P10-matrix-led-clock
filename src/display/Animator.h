#pragma once

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <string>
#include <vector>
#include <cmath>
#include "display/P10Driver.h"
#include "display/FontRenderer.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define ANIMATE_DURATION 500 // ms
#define MAX_ANIMATE_BATCH 10

class Animator {
public:
    struct AnimationTask {
        uint x{0};
        uint y{0};
        std::string fromText;
        std::string toText;
        bool mini{false};
        uint delay{0}; // Delay before starting this specific item's sliding within the batch (ms)
    };

    struct AnimationBatch {
        std::vector<AnimationTask> tasks;
    };

private:
    P10Driver* driver{nullptr};
    FontRenderer* fontRenderer{nullptr};

    std::vector<AnimationTask> currentBuildingBatch;
    QueueHandle_t batchQueue{NULL};
    TaskHandle_t workerTaskHandle{NULL};
    bool isInitialized{false};

    // Render string to 2D bool vector (0-pixel spacing)
    FontRenderer::Bitmap renderStringToBitmap(const std::string& text, bool mini);

    // Sine Ease-In-Out formula: t in [0.0, 1.0]
    static float easeInOutSine(float t) {
        return -(std::cos(M_PI * t) - 1.0f) / 2.0f;
    }

    // FreeRTOS worker thread entry
    static void animationWorkerTask(void* pvParameters);
    void processBatch(const AnimationBatch& batch);

public:
    Animator() = default;
    ~Animator();

    void init(P10Driver* p10Driver, FontRenderer* fontRenderer);

    // Adds animation task to current building batch
    void animate(uint x, uint y, std::string fromText, std::string toText, bool mini, uint delay);

    // Seals current batch and pushes to worker queue
    void startAnimate();
};