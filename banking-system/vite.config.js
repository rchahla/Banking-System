import { defineConfig } from "vite";
import react from "@vitejs/plugin-react";

export default defineConfig({
  plugins: [react()],
  server: {
    proxy: {
      "/api": {
        target: "http://34.130.214.64:3000", // Express backend
        changeOrigin: true,
      },
    },
  },
});
