# 4.5.6
### Addition
- Port to Geode 5 @ gd 2.208
- Along with event rewritten (it should work the same as before)
- Better blur effects (Thanks to TheSillyDoggo's [Blur API](https://github.com/TheSillyDoggo/Blur-API))
- Better warn page, the warn content image is removed to minimize mod package size.
- Teleport line fix and Ghost Trail fix now works without master switch ON. (But sure you should check their own options.)
- Particle effect in title bar will not go outside the round corner now. (not my work, thanks to NineSlice invention)
### Known Issues
- More Icons mod is not ported yet, current package will have compatibility issues with More Icons loaded in the future (won't crash your game though)
- Blur background acts not properly in white background, and it is not working on Apple devices.
### ToDo
- New Logic
- More modes and more free options
- Add Solid Wave Trail option (soon)

# 3.5.6
### Bugfix
- Fix crash with Icon Kit;
### Note
- Icon Kit is using a shader i guess, my mod will not work properly on playyer icons' sprites with Icon Kit. And ghost trail will be disabled under Robot / Spider mode (w/ icon kit loaded), I don't know how to fix that for now. In a word, only crash is fixed.
- Chroma Icons is not a completed / dead project, but updating this need time and it is not coming soon :/

# 3.5.5
### Update
- Bump Happy Textures Version to 2.0.0 as the requirement;
- New banner in about.md (planned to release in 3.6.0)

# 3.5.4
### Bugfix
- Fix ghost trails issue when flipped gravity or going upside down

# 3.5.3
### Bugfix
- Fix colors messing up with Separate Dual Icons loaded

# 3.5.2
### Bugfix
- fix blur effect breaking down in iOS (thanks hiimjasmine)

# 3.5.1
### Addition
- iOS support (I donot promiss this mod works well there, also happy textures has ios support when?)
- Add vehicle ghost trail option (not working in legacy ghost trail mode)
### Bugfix
- Fix cube ghost trails with vehicle looks as giant as cube mode  

# 3.5.0
### Addition
- A bit Main Menu Title change;
- Pause menu entry is now optional.
### Bugfix
- Default color on p2 icon issue should really get fixed this time (such a careless mistake)  
- Fix ghost trail missing texture bug when icon from More Icons applied (I really cannot get how that's happening, but it's fixed after all)
  
# 3.4.4
### Bugfix
- Fix default color on p2 icon issue (But failed)  
  
# 3.4.3
### Bugfix
- Fix mac crash (Thanks @hiimjustin000)
  
# 3.4.2
### Bugfix
- Fix default color sometimes may be slightly different from it should be  
- Remove meaningless error log  
  
# 3.4.1
### Bugfix
- Fix switch button crash (so late)  
  
# 3.4.0
### Bugfix
- Globed compatible, to chroma remote players is a bit crash risky so I don't dare to make it optional for now  
- Chroma Speed is now optional to sync timewarp value or ignore timewarp (sync real world clock)  

# 3.3.2
### Bugfix
- Ghost Trail chroma supports mac now (again thanks to @hiimjustin000)  

# 3.3.1
### Bugfix
- Add warning page sprite image (I miss that)  
- Some text polish  
  
# 3.3.0
Hoping this release will never crash our geometry dash, best wishes
### Addition
- Ghost Trail chroma (except mac for now)
- Ghost trail generator rewrite fix (optional) (except mac for now)
- Some perhaps useful options
### Bugfix
- Better UI text, to make the mod menu more user-friendly
- in-game playing more safe (I hope)
- Warning Page rebuilt (avoid possible crash)
### ToDo
- More Color pick options
- More chroma pattern options (chromatic phase, brightness, ...)
  
# 3.2.0-beta.4
- spider teleport line on Mac comes (thanks to @hiimjustin000)  
- Fix white robot / spider sprite chroma crashing the game (I hope)  
- Fix some spelling issues  
- Fix some other issues  

# 3.2.0-beta.3
### Bugfix
- Fix the *Interface* spelling issue (batch replace mistake)  
- Fix menu stuck when hit esc on menu launch  

# 3.2.0-beta.2
### Bugfix
- Fix the bug that Mac version banned Wave Trail rather than spider teleport line (stupid bug)  
- Better ReadMe  
  
# 3.2.0-beta.1
Finally new project worths being submitted to Geode index...
### Additions
- White sprite chroma;
- Spider Teleport Line returns (except mac);
- Spider teleport line position align fix;
- Separately config effect chroma patterns for different gamemode icons (advanced mode only)
### Bugfix
- Separate Dual Icons perfectly compatible;
- Best option of Progress chroma works now;
- More stable chroma engine;
- Better gradient logic;
- More useful options;
- Less bugs & Optimized codes.  
  
# 3.2.0-alpha.2
### Additions
- Chroma Engine constructed, now it should really chroma your icons;
- Menu Preview: preview effect in mod menu (it's optional);
### Bugfix
- Fix crash without More Icons installed, this bug is so stupid;
- Fix display issue of frames in mod menu ~~(by adding Happy Textures as required dependency)~~
- Optimized mod menu, more effects and more stable;
- Optimized codes.

# 3.2.0-alpha.1
Man! What can I say?
### Notice
- Windows Only
- This is only a pre-release of menu UI design, this release will no sense chroma your icons in-game
- This is not final UI conclusion. Will probably polish it later
- May perhaps crash your game despite max effort taken to avoid such tragedy. Personally suggest relaunch your game and save your game data before flirting with current menu

#### Legacy Chroma Icons Below
# 2.2.0
Besides Geode 4.x port, there are also:
### New element
Chroma Dash fire, even spider teleport line if you want!
### UI Redesign
#### Easy mode
Added Easy mode for most cases we don't expect seperate colors for icon status (works just like the old Apply to Common)
#### More Animation
Integrate the icon select menu and the config menu with more Animation. Hoping them look better
#### Self Made Effect Icons
Wave Trail, Particles, dash fire,...
#### Chroma State Preview
Preview your chromatic Icons directly in setup Menu!
### Bugfix
Confirmed everywhere works properly before release

# 1.1.0
### Bugfix
#### 1.Fix Robot/Spider Glow Not Working Bug!
How stupid can it be to chroma dynamic icons?
#### 2.Fix some visual bugs
~~I removed the title image of about.md~~  
I fixed the issue that the title image of about.md displays unproperly in Medium Quality mobiles.
### New stuffs
#### 1.Chromatic icons in editor playtest!
Considering capability issues, applying to editor playtest is optional, you can disable it individually.
#### 2.Phase options!
- For now you can decide your secondary&glow color align with main color (keep the same) or keep a constant phase distance (keep different)  
- Also able to decide Player 2 color phase aligns with Player 1 or not
#### 3.One-time Pointercrate Submission Notify
Only take effects in the first time you enter the mod menu in playing an extreme demon (Yes I have to do this)
#### 4.Copy & Paste Color in Static/Gradient Menu
Copy a color and quick paste it somewhere else!
#### 5.Some Visual Updates to options menu
ScrollLayer option menu
Description button of the *Apply to Common* option (in case some guys don't understand what does this option do)
Switch Option Color (simple and cool)
### To Do
- More than 2 colors gradient
- Set an icon effect set for single level
- Flash pulse
- Quick Settings
- Main Menu Effect Preview (perhaps?)
- Pick color from Robtop's official color set

# 1.0.0
Initial Release  
Quite many bugs! But at least it's here