## 🚀 1. Branch Structure
We use three branch types:
    main        → stable, completed features
    dev         → active development branch
    feature/*   → individual work branches of each teammate

Example:
    feature/nhan-rtsp
    feature/minh-rtp
    feature/linh-video
------------------------------------------------------------------------

## 👥 2. Team Workflow (Simple & Effective)
### Every Day
-   Pull latest `dev`:
        git checkout dev
        git pull
### Create a feature branch
    git checkout -b feature/my-feature

### Do your work and commit
    git add .
    git commit -m "feat: implement SDP response"

### Push your feature branch
    git push -u origin feature/my-feature

### Open a Pull Request (PR) → into `dev`
-   Go to GitHub\
-   Open PR: **feature/my-feature → dev**\
-   Add a clear description

### Teammate reviews the code
At least **one teammate** should review your PR.

### Merge into `dev`
Only when: - Reviewer approves\
- Code builds successfully

### Release to `main`
When the team finishes a milestone:
    dev → main
------------------------------------------------------------------------

## 🧪 3. Commit Message Style
We follow simple conventional commits:
-   `feat:` new feature\
-   `fix:` bug fix\
-   `refactor:` code cleanup\
-   `docs:` documentation updates

Examples:
    feat: add RTSP SETUP handler
    fix: correct RTP sequence number increment
    refactor: clean up VideoStream class

------------------------------------------------------------------------

## 📄 4. Pull Request Rules
Every PR should include:
-   **What** you changed\
-   **Why** you changed it\
-   **How to test it**
Example:
    ### What
    - Added DESCRIBE response with SDP
    - Implemented basic MJPEG RTP packetization

    ### Why
    - Needed for ffplay to connect properly

    ### How to test
    ffplay rtsp://127.0.0.1:554/stream
------------------------------------------------------------------------

## 🧹 5. Project Do's and Don't
### ✔ DO commit:
-   .cpp / .h source code\
-   CMakeLists.txt\
-   README\
-   scripts or configs\
-   documentation

### ❌ DO NOT commit:
-   build/\
-   .vs/\
-   .exe, .dll, .obj, .pdb\
-   temporary files
Ensure `.gitignore` filters out all build artifacts.
------------------------------------------------------------------------

## 📦 6. Recommended Folder Structure
    project/
     ├── server/
     ├── client/
     ├── common/
     ├── CMakeLists.txt
     ├── README.md
     └── .gitignore
-----------------------------------------------------------------------

## 🙌 8. Team Roles (Optional)
- **Dinh Nhan** --- Server && ServerWorker\
- **Thien Nhan** --- RTP packetization & VideoStream\
- **Quoc Hoc** --- Client && ClientWorker\