# ESP32 WiFi Health Monitor Dashboard

A modern frontend for monitoring ESP32 devices in real time. This dashboard connects to Firebase Realtime Database, displays live Wi-Fi and device health telemetry, and provides a clean, responsive interface for managing multiple devices from a single view.

This project is designed to be open, approachable, and easy to extend for the community. It works alongside the ESP32 firmware in this repository to give you a complete end-to-end monitoring solution.

## Overview

The dashboard provides:

- Real-time device monitoring
- Wi-Fi health and connection status views
- Device selection and telemetry browsing
- Authentication-backed access to the monitoring interface
- Clean, lightweight frontend built with Vite and vanilla JavaScript

It is intended for hobbyists, makers, students, and developers who want to build or learn from a practical IoT monitoring workflow using ESP32 devices and Firebase.

## Features

- Live data from Firebase Realtime Database
- Session-based login and protected access
- Device overview with health and telemetry panels
- Wi-Fi connectivity monitoring
- Theme toggle for improved readability
- Responsive layout optimized for desktop and browser-based dashboards
- Fast static frontend using Vite

## Tech Stack

- Vite
- Vanilla JavaScript
- Firebase Authentication
- Firebase Realtime Database
- HTML and CSS

## Project Structure

```text
dashboard/
├── index.html              # Landing / app bootstrap page
├── login.html              # Sign-in page
├── dashboard.html          # Main monitoring dashboard
├── package.json            # Vite and dependency config
├── vite.config.js          # Build and server configuration
├── src/
│   ├── css/                # Styling for all pages
│   └── js/
│       ├── app.js          # Application bootstrap
│       ├── auth.js         # Authentication helpers
│       ├── dashboard.js    # Dashboard UI and telemetry logic
│       ├── devices.js      # Device handling logic
│       ├── firebase.js     # Firebase initialization
│       └── login.js        # Login form behavior
└── firebase.json           # Firebase hosting configuration
```

## Prerequisites

Before running the dashboard, make sure you have:

- Node.js 18+ recommended
- npm
- A Firebase project with Realtime Database enabled
- Firebase web app configuration values

## Quick Start

1. Open a terminal in the dashboard folder:

```bash
cd dashboard
```

2. Install dependencies:

```bash
npm install
```

3. Create a local environment file:

```bash
cp .env.example .env
```

If there is no .env.example file in your setup, create a new .env file manually.

4. Add your Firebase configuration:

```env
VITE_FIREBASE_API_KEY=your_api_key
VITE_FIREBASE_AUTH_DOMAIN=your_project.firebaseapp.com
VITE_FIREBASE_DATABASE_URL=https://your_project-default-rtdb.firebaseio.com
VITE_FIREBASE_PROJECT_ID=your_project_id
VITE_FIREBASE_STORAGE_BUCKET=your_project.appspot.com
VITE_FIREBASE_MESSAGING_SENDER_ID=your_sender_id
VITE_FIREBASE_APP_ID=your_app_id
```

5. Start the development server:

```bash
npm run dev
```

The app will typically be available at:

```text
http://localhost:5173
```

## Available Scripts

```bash
npm run dev      # run the Vite development server
npm run build    # create a production build
npm run preview  # preview the production build locally
```

## Firebase Setup

This dashboard expects a Firebase project with:

- Firebase Authentication enabled
- Firebase Realtime Database enabled
- Web app configuration available in the Firebase console

The app uses environment variables from the Vite runtime to initialize Firebase securely.

> Do not commit your real .env file to a public repository. Keep secrets out of version control.

## Deployment

This frontend is a static site and can be deployed with:

- Firebase Hosting
- Netlify
- Vercel
- Any static web host that supports SPA-style routing

If using Firebase Hosting, the included firebase.json is the starting point for deployment.

## Development Notes

- The dashboard is structured as a lightweight static application and is intentionally easy to read and extend.
- The frontend is designed to react to Firebase database updates without requiring a complex framework.
- Device data, health metrics, and network conditions are expected to be published to Firebase by the ESP32 firmware.

## Security Considerations

This project is intended for learning and local or private deployment. For production use, consider:

- Restricting Firebase database access with proper rules
- Enabling Firebase Authentication and role-based access
- Keeping secrets and credentials in environment variables
- Validating incoming telemetry data before it reaches the frontend

## Contributing

Contributions are welcome.

If you want to improve the project:

1. Fork the repository
2. Create a feature branch
3. Make a focused change
4. Test locally
5. Open a pull request with a clear explanation

## Community Use

This dashboard is a good starting point for:

- ESP32 monitoring dashboards
- Smart home telemetry dashboards
- Wi-Fi health and resilience tools
- IoT education and prototype projects

If you build on this project, we encourage you to share improvements back to the community.

## Acknowledgements

This dashboard was designed to complement the ESP32 firmware in this repository and demonstrates a practical path from embedded device data to a live web dashboard.
