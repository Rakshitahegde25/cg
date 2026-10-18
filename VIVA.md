# Computer Graphics Viva Voce Questions & Answers

**Project Title:** Interactive 2D/3D Smart Parking Visualization, Booking and Navigation System  
**Tech Stack:** Pure C (`C99`), OpenGL, FreeGLUT, GLU, MinGW-w64 GCC  

---

### Q1: What is the primary objective of this Computer Graphics project?
**Answer:** The objective is to design and implement a real-time, interactive 2D and 3D visualization, booking, and navigation system for a smart parking lot using C and OpenGL. It demonstrates key graphics principles (primitives, coordinate transformations, `gluLookAt` camera math, `glOrtho` & `gluPerspective` projections, dynamic lighting) combined with AI pathfinding (A* algorithm) and a non-blocking per-second countdown timer engine (`glutTimerFunc`).

---

### Q2: What graphics API and windowing toolkit were used?
**Answer:** Core **OpenGL** (Open Graphics Library) for rendering graphics primitives, **GLU** (OpenGL Utility Library) for quadrics (`gluCylinder`, `gluSphere`) and matrix utilities (`gluLookAt`, `gluPerspective`, `gluUnProject`), and **GLUT / FreeGLUT** (OpenGL Utility Toolkit) for window creation, mouse/keyboard callbacks, and timer event loops.

---

### Q3: Explain the difference between `glOrtho()` and `gluPerspective()`.
**Answer:** 
- `glOrtho(left, right, bottom, top, near, far)` creates an **Orthographic Parallel Projection** where parallel lines remain parallel and objects retain their size regardless of distance from the camera. Used for the **2D top-down map view**.
- `gluPerspective(fovy, aspect, zNear, zFar)` creates a **Perspective Projection** where distant objects appear smaller, mimicking natural depth perception. Used for the **3D environment view**.

---

### Q4: How is camera viewing implemented using `gluLookAt()`?
**Answer:** `gluLookAt(eyeX, eyeY, eyeZ, targetX, targetY, targetZ, upX, upY, upZ)` sets up the viewing matrix:
1. `eye`: Camera coordinates in world space.
2. `target`: Center point the camera focuses on.
3. `up`: Vector pointing upward (typically `(0, 1, 0)`).
In `camera.c`, orbital math computes `eyeX`, `eyeY`, and `eyeZ` dynamically based on `yaw`, `pitch`, and `distance` values controlled by arrow keys.

---

### Q5: How does Mouse Picking work in both 2D and 3D views using `gluUnProject()`?
**Answer:** When the user left-clicks the screen at `(mouseX, mouseY)`:
1. GLUT window coordinates are converted into OpenGL viewport space: `winY = viewport[3] - mouseY`.
2. `gluUnProject()` unprojects `(mouseX, winY, 0.0)` (near plane) and `(mouseX, winY, 1.0)` (far plane) using `GL_MODELVIEW_MATRIX` and `GL_PROJECTION_MATRIX` to construct a 3D ray.
3. Ray-plane intersection with ground $Y=0$ yields exact world coordinates $(g_x, g_z)$.
4. The system tests if $(g_x, g_z)$ falls inside any parking slot bounding box to select it.

---

### Q6: How is the Customer Details & Booking Wizard implemented in OpenGL?
**Answer:** It is implemented as a non-blocking modal state machine (`BookingState`) rendered directly in the OpenGL window:
- Step 1: Customer Name Entry (`Person` struct field `name`).
- Step 2: Phone Number Entry (`phone`).
- Step 3: Vehicle Plate Number Entry (`vehicleNumber`).
- Step 4: Vehicle Type Selection (`1. Car`, `2. Bike`, `3. SUV`, `4. EV Car`).
- Step 5: Accessible Eligibility Check (`1. YES`, `2. NO`).
- Step 6: Parking Duration Selection (`30 min`, `1 hr`, `2 hr`, `3 hr`, `5 hr`, or `T` for Test Timer).
- Step 7: Booking Summary Confirmation (`ENTER` to confirm & navigate).

---

### Q7: What is the C structure used to store customer and vehicle details?
**Answer:**
```c
typedef struct {
    char name[50];
    char phone[20];
    char vehicleNumber[20];
    char vehicleType[20];   // Car, Bike, SUV, EV Car
    int eligibleAccessible; // 1 = YES, 0 = NO
} Person;
```
Each `ParkingSlot` structure contains a `Person customer` member to store these details upon booking confirmation.

---

### Q8: How does the Accessible / Disabled Parking eligibility check work?
**Answer:** When booking a Disabled/Accessible slot (`TYPE_DISABLED`), the system asks: `"Eligible for Accessible Parking? 1. YES  2. NO"`. If `2. NO` is selected, the booking is blocked with the alert: `"Normal vehicles cannot book accessible slots!"`.

---

### Q9: How is the non-blocking per-second Parking Timer implemented using `glutTimerFunc()`?
**Answer:** The system uses two separate timer loops:
1. `glutTimerFunc(16, timerCB, 0)` running at 60 FPS for smooth vehicle rendering and animation.
2. `glutTimerFunc(1000, perSecondTimerCB, 0)` executing every 1000 ms to decrement remaining parking times (`remainingTime--`), EV charging times (`chargingRemaining--`), and reservation timers (`reservationRemaining--`).
Because timer ticks update state without blocking execution, the OpenGL application remains completely responsive.

---

### Q10: What happens when less than 10 minutes or 5 minutes remain on a parking timer?
**Answer:**
- **< 10 minutes (600s)**: Visual warning banner: `"WARNING: Parking time ending soon for Slot A05!"`
- **< 5 minutes (300s)**: Urgent warning banner: `"URGENT: Parking time expires soon for Slot A05!"`

---

### Q11: What happens when a parking timer reaches zero?
**Answer:**
1. Slot status changes to `STATUS_TIME_EXPIRED`.
2. Slot visual surface changes to **Flashing Magenta / Amber**.
3. Dashboard banner displays: `"PARKING TIME EXPIRED for Slot A05! Customer: Rahul Kumar"`.
4. Vehicle remains parked in the slot and another vehicle is NOT allowed to park there until released.

---

### Q12: How does the Slot Release (`R` key) functionality work?
**Answer:** When `R` is pressed on an occupied or expired slot:
1. Parked car is removed (`drawCar3D`/`drawCar2D` suppressed).
2. Customer details are cleared (`hasCustomer = 0`).
3. Countdown timer is reset (`remainingTime = 0`, `timerActive = 0`, `timeExpired = 0`).
4. Slot status changes back to `STATUS_AVAILABLE`.
5. Dashboard HUD counters (Available, Occupied, Time Expired, Active Bookings) update automatically.

---

### Q13: What is the purpose of the Test Timer (`T` key) mode?
**Answer:** For quick academic laboratory demonstrations, pressing `T` allows selecting a **30 Seconds** or **60 Seconds** test timer instead of multi-hour durations. This allows demonstrating countdown warnings, timer expiration (`TIME EXPIRED`), and slot release (`R`) in under a minute.

---

### Q14: How does Pause / Resume (`P` key) work?
**Answer:** Pressing `P` toggles `timerActive` for parking slot countdown timers and `isPaused` for moving navigation vehicles. Switching between 2D and 3D views does NOT pause or reset active timers.

---

### Q15: How does EV Charging Timer management work?
**Answer:** EV slots (`TYPE_EV`) maintain a `chargingRemaining` countdown timer alongside the main parking timer. The HUD displays `Charging Time: 1 Hour | Remaining: 00:42:15 | Status: CHARGING`. When `chargingRemaining == 0`, it displays `CHARGING COMPLETE`.

---

### Q16: How does the A* Pathfinding Algorithm calculate vehicle navigation routes?
**Answer:** Road networks are modeled as a weighted graph of nodes (`navigation.c`). The A* algorithm finds the optimal path with minimum total cost $f(n) = g(n) + h(n)$:
- $g(n)$: Actual road distance from entrance node to node $n$.
- $h(n)$: Euclidean distance heuristic $\sqrt{dx^2 + dz^2}$ from node $n$ to the target slot aisle node.
The path coordinate list is rendered using a bright cyan `GL_LINE_STRIP`.

---

### Q17: How is vehicle animation executed smoothly around corners?
**Answer:** During each frame in `updateVehicle(float deltaTime)`:
1. Vehicle computes displacement vector $(dx, dz)$ to the next path waypoint.
2. Desired heading angle is calculated via `atan2f(dx, dz) * 180 / PI`.
3. Angle difference is smoothly interpolated per frame (`angle += diff * 10 * deltaTime`) to prevent sudden orientation snapping.

---

### Q18: Explain the color coding scheme used for parking slots.
**Answer:**
- **Green**: `AVAILABLE` standard slot
- **Red**: `OCCUPIED` slot with parked car
- **Yellow**: `RESERVED` slot
- **Blue**: `ACCESSIBLE / DISABLED` slot
- **Purple**: `EV CHARGING` slot
- **Flashing Magenta**: `TIME EXPIRED` slot
- **Thick Yellow Outline**: `SELECTED` slot

---

### Q19: What information does the Customer / Booking Info view (`I` key) display?
**Answer:** Pressing `I` on an occupied slot opens an overlay displaying:
- Customer Name, Phone, Vehicle Plate Number, Vehicle Type
- Slot ID, Slot Type (Normal, EV, Accessible)
- Total Duration, Remaining Countdown Time, Status (`OCCUPIED` / `TIME EXPIRED`)

---

### Q20: How are statistics updated dynamically on the Dashboard HUD?
**Answer:** `updateParkingStats()` iterates through all 36 slots in `slots[MAX_SLOTS]` and counts slots matching `STATUS_AVAILABLE`, `STATUS_OCCUPIED`, `STATUS_RESERVED`, `STATUS_TIME_EXPIRED`, `TYPE_EV`, and `TYPE_DISABLED`. Every HUD text string is updated automatically whenever slot state changes.
