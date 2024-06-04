const { contextBridge, ipcRenderer } = require('electron');

contextBridge.exposeInMainWorld('api', {
    startTracking: () => ipcRenderer.send('start-tracking'),
    stopTracking: () => ipcRenderer.send('stop-tracking'),
    onStateChange: (callback) => ipcRenderer.on('state-change', (event, state) => callback(state)),
});