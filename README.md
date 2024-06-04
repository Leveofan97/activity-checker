# Activity Watcher (Only Windows)
Этот проект - пример использования node модуля основанного на логике C++ кода.
По сути это дополнение написанное С++ для использования в node js.

У браузера, как и у electron приложения нет доступа к api на уровне операционной системы. 
В частности нет доступа к отслеживанию движений мыши и нажатий клавиатуры вне окна браузера/electron приложения.

Используя node модуль реализованный на С++ с доступом к win API у нас появляется возможность отслеживать изменение активности пользователя.
В частности под признаком активности подразумевается наличие движений мыши или нажатий клавиш.

Кроме этого в модуле реализована возможность настройки времени бездействия и времени периода проверки.

## Установка

Данная версия была собрана на [Node.js](https://nodejs.org/) v18.0.0 .

Установка зависимостей.

```sh
npm install

npm install node-addon-api
```

## Сборка модуля локально (из файлов проекта с помощью node-gyp)
```sh
npm node-gyp configure

npm node-gyp build
```

Обратите внимание на файл binding.gyp в котором прописана конфигурация (пути) собираемых файлов.

## Установка пакета из репозитория NPM

[activity_watcher](https://www.npmjs.com/package/activity_watcher)

```sh
npm i activity_watcher
```

## Использование
```
const activityChecker = require('activity_watcher');

    ...
    сonst inactivityThreshold = 10000; // Порог бездействия в миллисекундах
    const checkInterval = 2000; // Период проверки в миллисекундах
    
    //Запускает отслеживание  
    activityChecker.startTracking((state) => {
        console.log("Состояние: ", state);
    }, inactivityThreshold, checkInterval);
    ...
    
    //Останавливает отслеживание
    activityChecker.stopTracking();
```

## ToDo

- Поддержка Linux