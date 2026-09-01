import { defineConfig } from "vite";
import { resolve } from "path";

export default defineConfig({
    root: ".",

    publicDir: false,

    server: {
        host: "0.0.0.0",
        port: 5173
    },

    preview: {
        host: "0.0.0.0",
        port: 4173
    },

    build: {
        outDir: "dist",
        emptyOutDir: true,

        rollupOptions: {
            input: {
                main: resolve(__dirname, "index.html"),
                login: resolve(__dirname, "login.html"),
                dashboard: resolve(__dirname, "dashboard.html")
            }
        }
    }
});