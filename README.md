
# 🎓 Classroom Scheduler System

![C](https://img.shields.io/badge/C-100%25-blue)
[![License: MIT](https://img.shields.io/badge/License-MIT-green.svg)](https://opensource.org/licenses/MIT)
![Status](https://img.shields.io/badge/Status-80%25%20Complete-orange)

> An intelligent timetable management system for academic institutions built with C

## ✨ Features

- **Automated Scheduling**
  - Conflict-free timetable generation
  - Multi-section support (A/B/C/D)
  - Dynamic lecture insertion/removal

- **Faculty Management**
  - Workload analysis with overload alerts
  - Teacher swap functionality
  - Undo/redo operations

- **Data Persistence**
  - Save/load timetables in `.txt` format
  - Timestamped exports
  - Section-wise breakdowns

## 🛠️ Installation


# Clone repository 
git clone https://github.com/SaiyamTuteja/C-Based-Classroom-Scheduler.git

# Compile
gcc -o scheduler main.c utils.c

# Run


## 🖥️ Usage

1. View Timetable
2. Swap Teachers
3. Change Class Section
4. Teacher Workload Analysis
5. Assign New Teacher
6. Save Timetable
7. Load Timetable
8. Exit

## 📂 Project Structure

```
classroom-scheduler/
├── src/
│ ├── main.c # Core application entry point
│ ├── scheduler.h # Main header with struct definitions
│ ├── scheduler.c # Core scheduling algorithms
│ ├── file_io.c # Timetable save/load operations
│ └── utils.c # Helper functions
├── docs/
│ ├── MANUAL.md # User documentation
│ └── DESIGN.md # System architecture
├── samples/
│ ├── timetable_A.txt # Example timetable for Section A
│ └── faculty_load.txt # Sample workload report
├── tests/
│ ├── test_scheduler.c # Unit tests
│ └── test_data/ # Test cases
├── Makefile # Build configuration
└── README.md # Project documentation
```

## 🧑‍💻 Development Team

| Team Member        | Contribution                  |
|--------------------|-------------------------------|
| Priyanshu Solanki  | Data Structures (Stack/List)  |
| Saiyam Tuteja      | Timetable Initialization      |
| Ayush Pal          | Conflict Resolution          |
| Mintu              | File I/O Operations          |

## 📜 License

MIT © 2024 Classroom Scheduler Team


### Key Features:
- Modern badge styling for visual appeal
- Clear feature categorization
- Simple installation/usage instructions
- Team attribution table
- Professional formatting with code blocks
- License information
- Status transparency

Would you like me to add any additional sections like:
- Screenshots (once GUI is available)
- Detailed API documentation
- Contribution guidelines
- Roadmap for future versions?