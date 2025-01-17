</head>
<body>
    <img src="https://github.com/soycarlo99/dibujo/blob/main/logo.png?raw=true" alt="Program Logo" width="100" height="100">
</body>
</html>
# Dibujo

Dibujo is a drawing application with features like free drawing, rectangles, circles, and text input (can't change the size of the text yet). It also includes a color picker and supports background color changes. if you want to quickly show/draw something while screensharing or in person, you can quickly open this app via terminal and draw/write whatever you want to show/explain and close it easily with ```⌘ + q``` and continue with your work.

## Features

- Freehand drawing with adjustable brush thickness (arrow up & down)
- Draw rectangles, circles, and add text with preview options.
- A built-in color picker for brush colors.
- Background color cycling with a button or key shortcut (`B`).

## Installation

### Using Homebrew
```bash
brew tap soycarlo99/dibujo-tap
brew install dibujo
```

### Build From Source
Clone the repository and build the program manually:
```bash
git clone https://github.com/soycarlo99/dibujo.git
cd dibujo
make
./dibujo
```

## Usage

- Run the program with:
  ```bash
  dibujo
  ```
- Use the buttons or keyboard shortcuts to switch modes:
  - **Rectangle**: Button or `R`
  - **Circle**: Button or `C`
  - **Text**: Button or `T`

## Dependencies

- [SFML 2](https://www.sfml-dev.org/) (this is supposed to be installed automatically with the homebrew formula)

## Contributing

Contributions are welcome! Open an issue or submit a pull request.

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.
