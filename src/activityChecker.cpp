#include <napi.h>
#include <windows.h>
#include <thread>
#include <chrono>
#include <fstream>
#include <mutex>
#include <string>

std::mutex lockObj;
std::thread inactivityThread; //Поток для проверки бездействия.
bool isActive = true;
bool isRunning = false;
Napi::ThreadSafeFunction tsfn; //Потокобезопасная функция для вызова JavaScript функций из других потоков.

/*
    Создает структуру LASTINPUTINFO.
    Инициализирует cbSize размером LASTINPUTINFO.
    Вызывает функцию GetLastInputInfo для получения времени последнего ввода.
    Возвращает время простоя в миллисекундах.
*/
DWORD GetIdleTime() {
    LASTINPUTINFO lastInputInfo;
    lastInputInfo.cbSize = sizeof(LASTINPUTINFO);
    if (!GetLastInputInfo(&lastInputInfo)) return 0;
    return GetTickCount() - lastInputInfo.dwTime;
}

/*
    Функция CheckInactivity
    Работает в отдельном потоке, пока isRunning = true.
    Вызывает GetIdleTime для получения времени бездействия.
    Проверяет, превысило ли время простоя, порог inactivityThreshold, и вызывает соответствующие функции обратного вызова (JavaScript).
    Ожидание checkInterval (Период проверки в миллисекундах) перед следующей проверкой.
*/

void CheckInactivity(Napi::Env env, Napi::Function jsCallback, int inactivityThreshold, int checkInterval) {
    while (isRunning) {
        DWORD idleTime = GetIdleTime();
        {
            std::lock_guard<std::mutex> lock(lockObj);
            if (idleTime >= static_cast<DWORD>(inactivityThreshold) && isActive) {
                isActive = false;
                tsfn.NonBlockingCall([](Napi::Env env, Napi::Function jsCallback) {
                    jsCallback.Call({ Napi::String::New(env, "inactive") });
                });
            } else if (idleTime < static_cast<DWORD>(inactivityThreshold) && !isActive) {
                isActive = true;
                tsfn.NonBlockingCall([](Napi::Env env, Napi::Function jsCallback) {
                    jsCallback.Call({ Napi::String::New(env, "active") });
                });
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(checkInterval));
    }
}

/*
    Функция StartTracking
    Получает окружение выполнения env и JavaScript функцию обратного вызова jsCallback.

    (Объект Napi::Env,предоставляет доступ к среде выполнения Node.js из расширения C++.
     Обеспечивает создание объектов JavaScript и вызова функций JavaScript из C++.)

    Устанавливает флаг isRunning в true и создает ThreadSafeFunction.
    Запускает поток inactivityThread, который выполняет функцию CheckInactivity.

    На вход получает объект JavaScript вида

    ((state) =>
        {
            event.sender.send('name event', state); // отправляемый name event регистрируется из логики node js
        },
            inactivityThreshold, // Порог бездействия в миллисекундах
            checkInterval) // Период проверки в миллисекундах
*/

void StartTracking(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    Napi::Function jsCallback = info[0].As<Napi::Function>();
    int inactivityThreshold = info[1].As<Napi::Number>().Int32Value();
    int checkInterval = info[2].As<Napi::Number>().Int32Value();

    {
        std::lock_guard<std::mutex> lock(lockObj);
        if (isRunning) return;
        isRunning = true;
    }

    tsfn = Napi::ThreadSafeFunction::New(env, jsCallback, "TSFN", 0, 1);

    inactivityThread = std::thread(CheckInactivity, env, jsCallback, inactivityThreshold, checkInterval);
}

/*
    Функция StopTracking:
    Устанавливает флаг running в false.
    Ожидает завершения потока inactivityThread (если он активен).
    Освобождает ThreadSafeFunction.
*/
void StopTracking(const Napi::CallbackInfo& info) {
    {
        std::lock_guard<std::mutex> lock(lockObj);
        if (!isRunning) return;
        isRunning = false;
    }
    if (inactivityThread.joinable()) {
        inactivityThread.join();
    }
    tsfn.Release();
}

/*
    Функция Init:
    Регистрация функций StartTracking и StopTracking в объекте exports, который экспортируется модулем.
*/

Napi::Object Init(Napi::Env env, Napi::Object exports) {
    exports.Set(Napi::String::New(env, "startTracking"), Napi::Function::New(env, StartTracking));
    exports.Set(Napi::String::New(env, "stopTracking"), Napi::Function::New(env, StopTracking));
    return exports;
}

/*
    Макрос NODE_API_MODULE регистрирует функцию инициализации Init как точку входа модуля. Имя модуля в Node.js будет "activityChecker".
*/

NODE_API_MODULE(activityChecker, Init)