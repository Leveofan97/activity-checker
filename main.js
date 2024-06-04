const { app, BrowserWindow, ipcMain} = require('electron');
const path = require('path');
const activityChecker = require('./build/Release/activityChecker');

let mainWindow;

const createWindow = () => {
    mainWindow = new BrowserWindow({
        width: 800,
        height: 600,
        webPreferences: {
            preload: path.join(__dirname, 'preload.js'),
            contextIsolation: true,
            enableRemoteModule: false,
            nodeIntegration: false,
        },
    });

    mainWindow.loadFile('index.html');
    mainWindow.on('closed', () => {
        mainWindow = null;
    });
}

app.on('ready', createWindow);

app.on('window-all-closed', function () {
    if (process.platform !== 'darwin') {
        app.quit();
    }
});

app.on('activate', function () {
    if (mainWindow === null) {
        createWindow();
    }
});
ipcMain.on('start-tracking', (event) => {
    const inactivityThreshold = 10000; // Порог бездействия в миллисекундах
    const checkInterval = 2000; // Период проверки в миллисекундах
    activityChecker.startTracking((state) => {
        event.sender.send('state-change', state);
    }, inactivityThreshold, checkInterval);
});

ipcMain.on('stop-tracking', () => {
    activityChecker.stopTracking();
});