# Product Mission

## Pitch

SteambandRedux is a modernized roguelike game that helps gamers enjoy classic dungeon-crawling gameplay using Xbox controllers and Steam platform features by providing seamless controller support, Steam integration, and a polished gaming experience.

## Users

### Primary Customers
- **Controller-first gamers**: Players who prefer gamepad input over keyboard for roguelikes and want a comfortable couch gaming experience
- **Steam platform users**: Gamers who want classic roguelikes available on Steam with achievements, cloud saves, and Steam overlay support
- **Angband veterans**: Existing players of Angband variants who want modernized controls and platform integration

### User Personas

**The Couch Gamer** (25-45)
- **Role:** Casual to hardcore gamer
- **Context:** Prefers playing games from the couch with a controller rather than at a desk with keyboard/mouse
- **Pain Points:** Most roguelikes require keyboard input, making them uncomfortable for extended play sessions. Lack of Steam integration means no achievements, cloud saves, or easy access through Steam library.
- **Goals:** Play classic roguelikes comfortably with a controller, track progress through Steam achievements, and have save games backed up to the cloud.

**The Steam Collector** (20-40)
- **Role:** Steam platform enthusiast
- **Context:** Prefers games available on Steam for unified library management, achievements, and social features
- **Pain Points:** Many classic roguelikes aren't available on Steam or lack modern Steam features. Missing Steam overlay makes it harder to chat with friends or take screenshots.
- **Goals:** Access classic roguelikes through Steam, earn achievements, and share progress with friends.

**The Classic Gaming Enthusiast** (30-50)
- **Role:** Long-time roguelike player
- **Context:** Grew up playing Angband and similar games, wants to continue enjoying them with modern conveniences
- **Pain Points:** Old codebases lack modern features like controller support and Steam integration. Build systems are outdated and difficult to maintain.
- **Goals:** Enjoy classic roguelike gameplay with modern controls and platform features while maintaining the authentic experience.

## The Problem

### Legacy Roguelikes Lack Modern Gaming Features

Classic roguelikes like Angband and its variants were designed for keyboard input and lack support for modern gaming controllers. Additionally, they're not available on Steam, missing out on achievements, cloud saves, and the Steam overlay. This limits accessibility for gamers who prefer controllers and excludes them from the Steam ecosystem.

**Our Solution:** Modernize SteambandRedux with Xbox 360 controller support via XInput, integrate Steamworks SDK for full Steam platform features, and add robust logging and testing infrastructure for maintainability.

### Outdated Build Systems Hinder Development

The codebase uses ancient build tools (Borland C++ 4.5) that are incompatible with modern development workflows and libraries. This prevents integration of modern APIs and makes the codebase difficult to maintain or extend.

**Our Solution:** Migrate to CMake build system, enabling compatibility with modern compilers and easy integration of Steamworks SDK and XInput libraries.

## Differentiators

### Full Controller Support

Unlike most roguelikes that require keyboard input, SteambandRedux provides complete Xbox 360 controller support. Players can navigate menus, move through dungeons, and perform all game actions using only a controller. This results in a comfortable, accessible gaming experience suitable for extended play sessions.

### Native Steam Integration

Unlike ports that lack Steam features, SteambandRedux integrates Steamworks SDK natively, providing achievements, Steam overlay, cloud saves, and full Steam platform integration. This results in a polished Steam release that feels like a modern game rather than a retro port.

### Modernized Yet Authentic

Unlike complete rewrites that lose the classic feel, SteambandRedux modernizes the infrastructure while preserving the authentic Angband gameplay experience. This results in the best of both worlds: classic roguelike gameplay with modern platform features.

## Key Features

### Core Features
- **Xbox 360 Controller Support**: Complete gamepad control for movement, actions, and menu navigation, making the game accessible for controller-first gamers
- **Steam Platform Integration**: Native Steamworks SDK integration providing achievements, Steam overlay, cloud saves, and Steam API features
- **Modern Build System**: CMake-based build system enabling easy compilation, modern compiler support, and straightforward library integration

### Quality Features
- **Comprehensive Logging**: Robust logging system with multiple log levels, file output, and debug console support for troubleshooting and development
- **Testing Infrastructure**: Unit testing framework ensuring reliability and maintainability of new features and core functionality
- **Windows Optimization**: Optimized for Windows platform with native API usage and modern compiler optimizations

### Advanced Features
- **Configurable Controls**: Customizable controller mappings allowing players to tailor the experience to their preferences
- **Steam Achievements**: Achievement system integrated with Steam, rewarding players for milestones and accomplishments
- **Steam Cloud Saves**: Save game synchronization through Steam Cloud, ensuring progress is never lost

