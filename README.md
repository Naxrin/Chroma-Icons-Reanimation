![](./manual%20images/titlebg.png?raw=true)
# Chroma Icons Reanimate Project

So as you can see, former [Chroma Icons](https://github.com/Naxrin/Chroma-Icons) returns.  
In this new project, I rewrite almost the whole mod, not only the menu, but also the chroma engine.

Besides 2.2 port of **Mega Hack Icon Effect**, this mod aims to provide a complete solution on your player related sprites' color pattern.  
### Integrated Mod Menu
Enjoy your mod config in new designed integrated mysterious menu;  
### Chroma Separately for Gamemodes
Set them together in Easy-Mode, or customize various gamemode icons individually in Advanced-Mode!    
Switch Easy / Advanced mode by clicking the (toggler) button at the bottom left corner of Main Page.  
### Separate Riders
Optional to set cube riders' color follow their ship / ufo / jetpack vehicles or the cube's own color;  
### Ultimate Switch
Toggle ON / OFF the whole mod without having to relaunch your game!  
  
# WARNING
Regarding Pointercrate List Team's policy for Mega Hack v7's Icon Effects, some behaviors related with this mod, such as editing Icon Effects in playing a demon list level, is probably <font color = "red">**Not Allowed**</font> for record submission.  

## Also Note That:
- Despite max effort made on compatibility, This mod may very probably conflict with other related Icon Color modifier mods (e.g. Same Dual Color). I'm not willing to set them conflict to force you pick one, But you can take a try yourself;  
- Spider Teleport Line modifying is yet not supported on Mac ~~cuz PlayerObject::onSpiderJump(bool) is not implemented for Mac yet~~  
  
# What Does this mod do?
This mod adds its menu entrance button to:  
- **Icon Select Menu** *(GJGarageLayer)*
- **Pause Menu** *(PauseLayer)*
- **Level Editor Page** *(LevelEditorLayer)* (if the option Editor Test ON)  
so you can access the mod menu either in game menu or in playing a level.  
If you change any chroma pattern in a paused level, this mod will take effect once you return to play.  

## Easy Mode
For most cases, we do not need separated chroma patterns for different gamemodes. So yep understanding this mod just in easy mode is not that hard.  
  
### Check you are in Easy Mode
In main mod menu you see a button at the bottom left, that is the Easy - Advanced Switch. So here we keep it as easy face outlooking, then we are in Easy Mode.  
![](./manual%20images/Item%20Menu%20Guide.png)
  
### Enter Setup Menu
As shown above, you should see your player cube icon laying in the center ~~even earlier than the Easy/Adv mode switch~~.  
Yes then click the cube and you should see the **Setup Page**. Here you can modify your chroma pattern. Note that all your changes will take effect in the selected button icon immediately so you can preview your effect there.  
![](./manual%20images/Setup%20Menu%20Guide%20Easy.png)
  
### Set your Chroma Mode
For now this mod provides five modes for each sprite:  
#### Default
Just set the target sprite default as the mod is not loaded here.
- Note that this mod is aware of separate dual icons changes.
#### Static
Pick a color then the target sprite will stay constant here. Click the color channel sprite frame then the color page pops out, and you can pick the color here.  
- Drag the color picker, or drag the RGB sliders, or edit the RGB / Hex inputs, to set your color value;
- Just as Geode's color pick popup, the color page shows the original color and current color for you. But in Addition here, both of them are buttons, they pop up their Copy / Paste and Copy / Recover buttons. You know what they mean, or you just try them out.  
#### Chromatic
Just most favoured RGB cycle, let your target sprite's color hue gradients cyclic.  
- Set the saturation value to 50 if you prefer pastel like cycle.  
- will add phase offset, brightness and reverse cycle options in future update(?)  
#### Gradient
Pick two colors then the target sprite will cyclic gradient to each other.
- For now, set duty value to control which color displays more time in a cycle. In future update I may look into Photoshop like gradient bar.  
#### Progress
Pick a color for 0% progress and 100% progress, your target sprite will tint from the 0% color to the 100% color during your gameplay run.
- If the **Best Progress** toggler toggled, the target sprite color will keep a constant color regarding your current best progress on this level rather than your current percentage, so it will get closer to the 100% only when you make new progress or beat the level.
- Also note that platform levels only accept 0% or 100% as progress, you should know why.  
  
### Switch Sprite Channel
In the chapter above **target sprite** is frequently mentioned, actually you are just modifying main color sprite just now. So how to modify chroma patterns for the second color sprite, glow sprite and more? Yes you can click the arrows:  

This mod provides us with eight channels as target sprite:  
#### Main
The sprite colored main (first) color by default.
#### Second
The sprite colored second color by default.
#### Glow
Your glow outline.
#### White
The detail sprites of some unlock item icons, they keeps white regardless your preference in RobTop's color settings. (not all unlock items have such sprite).
#### Trail
Just the regular trail behind your player icon.
#### Dash Fire
Dash fire of you are currently acting orb dash.
#### TP Line
General spider jump and purple pad / orb (ring) teleport generates a effect line, that's what it points to.
#### Wave Trail
Wave only ~~if you donot know what it stands for plz go play Blast Processing~~
#### UFO Shell
Ufo Only, it's just the shell sprite of your "bird".

### Switch between Gamemodes / Effects
In Main Page, there are frame label buttons laying below your player icon, they work as effect shortcuts. They directly enter their effect modify page on click.  
You can either return to Main Page and click another icon / effect label, or just click the item label in the left of Setup Page.  
To avoid some mod menu logic mess, in modifying effects you can't switch to other channels.  
  
### Speed Slider
A set of speed option menu lays in Main Page, Set the chromatic / gradient cycle speed by dragging the slider or editing the text input or clicking the two arrows. Your changes take effect once you released the slider thumb, finished input or arrow clicked.  
The unit of the speed value you see in mod menu is Hertz, that means your icons chroma in a period time of 1/speed if speed > 0 and stay constant otherwise.  
  
### Switch Player
This mod allows to set other chroma patterns for dual player 2,  click the **Player 1/2** text label in Main Page or Setup Page to switch between.  
If you're too lazy to set chroma pattern twice but hoping to make p2 just duplicate p1's pattern, then the mod option **Same Dual Chroma Setup** is for you.
  
## Advanced Mode
If you just gonna stay at easy mode. you can just skip this and go to options. But...?  
Click the easy - adv mode switch button, then the button will switch to Extreme Demon face, and the single player icon in the center will turn to the nine gamemodes icons.  
Yes you're now in advanced mode.  
  
### Gamemode-Channel Sprite Matrix
Now that you are trying to understand advanced mode, this mod introduces a matrix-like icon sprite logic as follows:  
![](./manual%20images/chart.png)
In this chart, each cell marked circle represents to the sprite part for this gamemode, cells marked cross represents to nonsense (e.g. Robot mode doesn't need Wave Trail, so the cell at Row 6 Col 8 is marked cross), each cell marked circle corresponds to a set of chroma pattern (<code>ChromaSetup</code> in code), each chroma pattern saves the mode you picked, and their sub options (static color, saturation, best progress, etc).In playing the level, this mod loads the config row of your current gamemode, and your player icon sprite parts and effects will get chroma separately regarding the chroma pattern for them inside the loaded row. That's the basic logic of this mod.  
  
For the first row noted Icon, it represents to All gamemodes' common config for easy mode, and will no sense affect your player icons if you are in adv mode. Same for reverse, all config rows for individual gamemodes will no sense affect your in-playing icons in easy mode.  
![](./manual%20images/chart_mode.png)  

### Modify chroma setups in Advanced-Mode
Click a gamemode icon above or a effect label below, you enter the Setup Page and the gamemode / effect browser auto switchs to the tab you click (same as Easy Mode).  
![](./manual%20images/Setup%20Menu%20Guide%20Adv.png)
#### Switch between Gamemodes / Channel in Advanced Mode
If you are in a gamemode tab, the channel switch arrows let you go to another channel on click (almost the same as Easy Mode). Or you can click a effect tab to skip directly to the effect channel of current gamemode.  

Surely, now you are in advanced mode, you should know only wave have wave trail and only ufo has a shell, so clicking Wave Trail / UFO Shell tab will teleport you to wave / ufo mode meanwhile.  

If you are in a effect tab, channel switch arrows are now enabled for adv mode (except wave trail / ufo shell), but now that you are modifying in **effect** tab, the arrows bring you to the nearby gamemode instead of the nearby channel. Also if you jump to a gamemode tab, then the current channel will be reset to **Main**.  
![](./manual%20images/chart_modify.png)
  
#### Modify Chroma Setup
As for modifying chroma setups and how a chroma setup affects its corresponding (target) gamemode sprite, they are the same with easy mode.  
## Options
The gear button at the top right enters options menu on click, There are some mod options for you.  
#### Switch
Toggle ON/OFF the mod.  
If the mod is toggled OFF, it will no sense chroma your icons in playing, all other options will effect nothing. items in home page and setup page will display gray-white regardless the Preview Effect option.
### General Options
I don't know how to group them.  
  
### Phase Options
Some quick phase options.  
In future update I may set them to Advanced-Mode Only.  
  
### Menu Options
Customize appearance of the mod menu.  
  
## Claim
All stuff related to blur background in this mod are copy from TheSillyDoggo's [Blur BG](https://github.com/TheSillyDoggo/Blur-BG)  
Will pull request soon and set that as dependency if Blur BG is added to index later.  
  
## Contact me outside Github:
- Discord: Naxrin#6957 (recommended)  
- Twitter: @Naxrin19  
  
~~But why not directly create an issue here?~~
