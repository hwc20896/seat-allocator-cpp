# Seat Allocator v3

A powerful C++ desktop application for intelligent seat arrangement and shuffling with constraint-based allocation.

![C++](https://img.shields.io/badge/C%2B%2B-23-blue)
![Qt](https://img.shields.io/badge/Qt-6.x-blue)
![Platform](https://img.shields.io/badge/platform-Windows-lightgrey)

## 🎯 Features

- **Smart Seat Shuffling**: Advanced algorithm that rearranges seats while respecting complex constraints
- **Constraint-Based Allocation**: Support for multiple constraint types including:
  - Force/Forbid specific rows or columns
  - Prevent people from sharing rows/columns
  - Custom forbidden pairs
  - Optional fixed points and neighbor preservation
- **Multiple File Formats**: Import/export support for both CSV and XLSX (Excel) files
- **Visual Grid Display**: Clean Qt-based UI with color-coded cells
- **Sound Effects**: Audio feedback for shuffle operations
- **Pagination**: Navigate through multiple seating arrangements
- **Color Presets**: Import color schemes or disable as needed

## 📋 Requirements

### Runtime Requirements
- Windows 10/11 (64-bit)
- No additional dependencies required - standalone executable includes all necessary libraries

### Build Requirements (for developers)
- CMake 4.1+
- C++23 compatible compiler (GCC 13+ or MSVC 2022+)
- Qt 6.x (Core, Widgets, Multimedia)
- spdlog
- OpenXLSX
- CLion (recommended IDE)

## 🚀 Installation

### For End Users

1. Download the latest release from [Releases](https://github.com/YOUR_USERNAME/seat_allocator_v3/releases)
2. Extract the archive to your desired location
3. Run `seat_allocator_v3.exe`

### For Developers

```
bash
git clone https://github.com/YOUR_USERNAME/seat_allocator_v3.git
cd seat_allocator_v3
mkdir build && cd build
cmake .. -G Ninja
cmake --build . --config Release
```
## 💡 Usage

### Basic Workflow

1. **Import Data**: 
   - Click "Import from CSV" or "Import from XLSX"
   - Load your seating data (names, IDs, etc.)

2. **Configure Constraints** (Optional):
   - Click "Load Constraints"
   - Define rules in JSON format (see example below)

3. **Shuffle Seats**:
   - Click "Begin Shuffle" to generate new arrangements
   - Use navigation buttons to browse through generated grids
   - Toggle "Original Grid" to compare with initial arrangement

4. **Export Results**:
   - Export to CSV or XLSX format
   - Save your shuffled seating arrangements

### Constraint Configuration

Create a JSON file to define custom constraints:

```json
{
  "allow_fixed_points": false,
  "allow_original_neighbors": true,
  "diagonals_are_neighbors": false,
  "custom_forbidden_pairs": [
    ["Alice", "Bob"],
    ["Charlie", "David"]
  ],
  "constraints": [
    {"type": "ForceRow", "first": "Alice", "second": 1},
    {"type": "ForbidCol", "first": "Bob", "second": 3},
    {"type": "ForbidShareRow", "first": "Eve", "second": "Frank"}
  ]
}
```

#### Shuffle Settings

- **`allow_fixed_points`**: Allow people to stay in their original positions (default: `false`)
- **`allow_original_neighbors`**: Allow original neighbors to remain adjacent (default: `false`)
- **`diagonals_are_neighbors`**: Consider diagonal cells as neighbors (default: `false`)
- **`custom_forbidden_pairs`**: Pairs of people who cannot be adjacent

#### Constraint Types

- **`force_row`**: Force a person to sit in a specific row
    - `value`: Person's name
    - `index`: Row number (0-based)
- **`forbid_row`**: Prevent a person from sitting in a specific row
    - `value`: Person's name
    - `index`: Row number (0-based)
- **`force_col`**: Force a person to sit in a specific column
    - `value`: Person's name
    - `index`: Column number (0-based)
- **`forbid_col`**: Prevent a person from sitting in a specific column
    - `value`: Person's name
    - `index`: Column number (0-based)
- **`forbid_share_row`**: Two people cannot sit in the same row
    - `value1`: First person's name
    - `value2`: Second person's name
- **`forbid_share_col`**: Two people cannot sit in the same column
    - `value1`: First person's name
    - `value2`: Second person's name

### Color Presets Configuration

Create a JSON file to define color presets for grid cells based on regex patterns:

```json
{ 
  "^A.": "#FF5733", 
  "^B.": "#33FF57", 
  ".*[0-9]$": "#3357FF"
}
```

This example will:
- Color all names starting with "A" in red
- Color all names starting with "B" in green
- Color all names ending with a number in blue

**Note**: Regex patterns are case-sensitive. Use valid .NET regex syntax.

### File Format

#### CSV Format
```
Alice,Bob,Charlie
David,Eve,Frank
Grace,Heidi,Ivan
```
#### Excel Format
- Same structure as CSV, using spreadsheet cells

## 🏗️ Architecture

### Core Components

- **`GridShuffler`**: Main algorithm class handling constraint-based shuffling
- **`Constraints`**: Constraint definition and validation system
- **`MainWindow`**: Qt-based user interface
- **`Utils`**: Utility functions for file I/O and data processing

### Technology Stack

- **Language**: C++23
- **UI Framework**: Qt 6 (Widgets, Multimedia)
- **Logging**: spdlog
- **Excel Support**: OpenXLSX
- **Build System**: CMake 4.1 + Ninja
- **Audio**: Qt Multimedia (WAV sound effects)

## 🔧 Development

### Building from Source

```
bash
# Clone the repository
git clone https://github.com/YOUR_USERNAME/seat_allocator_v3.git

# Navigate to project directory
cd seat_allocator_v3

# Create build directory
mkdir cmake-build-release
cd cmake-build-release

# Configure with CMake
cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build .

# Executable will be at: cmake-build-release/seat_allocator_v3.exe
```

### Debug Build

```
bash
mkdir cmake-build-debug
cd cmake-build-debug
cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build .
```
Debug builds include enhanced logging via spdlog.

## 🎨 Sound Effects

The application includes audio feedback:
- `snd_pling.wav` - Success/completion sound
- `snd_shuffle_start.wav` - Shuffle beginning
- `snd_wheel_click.wav` - Navigation clicks

Sounds are located in the `sounds/` directory and can be customized.

## 📝 Examples

### Simple Seating Arrangement

Create a CSV file with names:
```csv
John,Jane,Bob,Mary
Tom,Sue,Mike,Lisa
```
Load it into the application and click "Begin Shuffle" to generate new arrangements.

### Complex Constraints

For wedding seating, prevent divorced couples from sitting together:
```json
{ 
  "shuffle_settings": { 
    "allow_fixed_points": false, 
    "allow_original_neighbors": true, 
    "diagonals_are_neighbors": false, 
    "custom_forbidden_pairs": [ 
      ["Alice", "Bob"], 
      ["Charlie", "David"] 
    ]
  }, 
  "constraints": [ 
    { 
      "type": "force_row", 
      "value": "Alice", 
      "index": 1
    }, 
    { "type": "forbid_col", 
      "value": "Bob", 
      "index": 3
    }, 
    { "type": "forbid_share_row", 
      "value1": "Eve", 
      "value2": "Frank"
    } 
  ]
}
```
## 🐛 Troubleshooting

### Common Issues

**Application won't start:**
- Ensure all DLL files are present in the installation directory
- Check if Visual C++ Redistributable is installed

**Constraints not working:**
- Verify JSON syntax is correct
- Check that names match exactly (case-sensitive)
- Ensure constraints are satisfiable (not contradictory)

**Import fails:**
- CSV files must use comma separators
- Excel files must be .xlsx format (not .xls)
- Remove special characters or formulas from cells

## 🤝 Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/AmazingFeature`)
3. Commit your changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request

## 📧 Contact

For questions or issues, please open an issue on the GitHub repository.

## 🙏 Acknowledgments

- Qt Framework for the excellent UI toolkit
- spdlog for fast C++ logging
- OpenXLSX for Excel file support
- Backtracking algorithm with MRV heuristic for constraint satisfaction

---

**Built with ❤️ using C++23 and Qt6**
