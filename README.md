# SCUpdootHelper
Overview

The Star Citizen Update Helper is a C++ program designed to assist users in managing their Star Citizen game installation by providing the ability to delete specific folders associated with the game. The program offers three main functionalities:

    Delete Star Citizen Folders: This option allows users to delete all folders within the %localappdata%\Star Citizen directory.

    Delete User Folders: This option recursively searches for and deletes "USER" folders within the specified Star Citizen installation directories.

    Backup Keybinds, Settings including keybinds, and stick settings

Features

    Confirmation Dialogs: The program includes confirmation dialogs to ensure that users want to proceed with potentially irreversible operations.

    Flexible Search: The program is designed to adapt to different installation locations, including secondary drives.

How to Use

    Delete Star Citizen Folders:
        Click the "Delete Star Citizen Folders" button.
        Confirm the operation in the pop-up dialog.

    Delete User Folders:
        Click the "Delete USER Folders" button.
        Confirm the operation in the pop-up dialog.
    
    Backup Keybinds, Settings
       Keep the checkbox checked to preserve your settings including keybinds, and stick settings
       
    Close:
        Click the "Close" button or hit the "X" to exit the program.

Prerequisites

    This program is designed for Windows environments.


How to Compile and Run

    The EXE is included if you are unable to compile
    
    The program is written in C++ and requires a compatible compiler (e.g., GCC, Visual C++ Compiler).

    Ensure that the necessary libraries (Qt, Windows SDK) are properly installed.

    Use the provided source code to compile the program.

Contributions

    Contributions are welcome! If you have suggestions, improvements, or discover any issues, please feel free to open an issue or submit a pull request.

    Thank you for all your help Alpine.

This program is open-source.
