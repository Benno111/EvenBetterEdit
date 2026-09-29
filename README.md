# <span style="display: flex; align-items: center; gap: .25em"><img src="logo.png" width="50"> BetterEdit</span>

> [!WARNING]
> **BetterEdit is no longer OFFICALY being developed!** HJfod has [retired from GD modding](https://hjfod.github.io/blog/so-long-and-thanks-for-all-the-fish.html). I'd recommend using alternatives like [Tinker](https://geode-sdk.org/mods/alphalaneous.tinker) instead. Thank you for the years of love and support <3

<span>
  <a href="LICENSE"><img alt="License Badge" src="https://img.shields.io/github/license/HJfod/BetterEdit?label=license&style=flat-square" /></a>
  &ensp;&macr;&ensp;
  <a href="https://github.com/HJfod/BetterEdit/issues"><img alt="GitHub Issues - Open" src="https://img.shields.io/github/issues/HJfod/BetterEdit?style=flat-square" /></a>
  <a href="https://github.com/HJfod/BetterEdit/issues"><img alt="GitHub Issues - Closed" src="https://img.shields.io/github/issues-closed/HJfod/BetterEdit?style=flat-square" /></a>
  &ensp;&macr;&ensp;
  <a href="https://github.com/HJfod/BetterEdit/pulls"><img alt="GitHub Pull Requests - Open" src="https://img.shields.io/github/issues-pr/HJfod/BetterEdit?style=flat-square" /></a>
  <a href="https://github.com/HJfod/BetterEdit/pulls"><img alt="GitHub Pull Requests - Closed" src="https://img.shields.io/github/issues-pr-closed/HJfod/BetterEdit?style=flat-square" /></a>
  &ensp;&macr;&ensp;
  <a href="https://github.com/HJfod/BetterEdit/actions/workflows/build.yml"><img alt="GitHub Actions Workflow Status" src="https://img.shields.io/github/actions/workflow/status/HJFod/BetterEdit/build.yml?style=flat-square" /></a>
</span>

<br>

A mod that makes the <a href="https://store.steampowered.com/app/322170/Geometry_Dash/">Geometry Dash</a> editor, well, <i>better</i>.

EvenBetterEdit also includes
[Auto-Options](https://github.com/BlueToadMakerr/Auto-Options). Its editor
button records player jump presses and releases as Options objects while a
playtest is running, with an optional group ID configured in the mod settings.
The separate **Automatic Object Defaults** feature can also enable Don't Fade,
Don't Enter, and High Detail whenever an object is placed.

EvenBetterEdit also restores the pre-2.2 **Level Copy** dialog. When duplicating
a local level, you can independently keep its attempt statistics and completion
progress in the new copy.

### Using Level Copy options

1. Open **Create > My Levels** and select the level you want to duplicate.
2. Press the level's **Copy** button. The **Level Copy** dialog appears before
   the duplicate is created.
3. Enable **Copy attempts and jumps** to retain attempts, jumps, clicks, and
   attempt time in the duplicate.
4. Enable **Copy normal and practice progress** to retain both completion
   percentages in the duplicate.
5. Press **Copy** in the dialog. The level itself is always duplicated; any
   statistics whose option is disabled start with the game's normal defaults.

### Upload copy settings

The level upload screen also has a **Copy Settings** button. Use it to choose
whether other players may copy the uploaded level:

* Disable **Allow level copying** to make the level non-copyable.
* Enable **Allow level copying** and **Free copy** to let anyone copy it without
  a password.
* Enable **Allow level copying**, disable **Free copy**, and enter a numeric
  password to require that password when the level is copied.

Press **Save** before uploading the level to apply the selected copy setting.

## :rocket: Installation

You can install BetterEdit through [Geode](https://geode-sdk.org). After installing Geode, simply search for the mod on the in-game browser, and click install.

**BetterEdit needs the following mods to also be installed:**

 * [NodeIDs](https://geode-sdk.org/mods/geode.node-ids)
 * [GMD API](https://geode-sdk.org/mods/hjfod.gmd-api)
 * [Level ID API](https://geode-sdk.org/mods/cvolton.level-id-api)
 * [Editor Tab API](https://geode-sdk.org/mods/alphalaneous.editortab_api)

## :beetle: Bug reports & feature suggestions

You can use [Issues](https://github.com/HJfod/BetterEdit/issues) to report bugs and suggest new features! Click [here](https://github.com/HJfod/BetterEdit/issues/new/choose) to open up a new issue.

Please use the correct templates for your issue - badly formatted issues will be closed.

| Issue template name | What it's for |
| ------------------- | ------------- |
| Bug Report          | Reporting a bug with the mod, such as some feature not working as expected, some buttons being misplaced, etc. |
| Crash Report        | Reporting a crash with the mod (i.e. when the game closes unexpectedly) |
| Suggestion          | Suggesting a new feature to be added to the mod / changes to an existing feature |

## :speech_balloon: Contact

BetterEdit has a [Discord server](https://discord.gg/rPvFW4jQTJ); this is where you can go if you need any further information, wish to ask questions, or anything else!

[<img alt="BetterEdit Discord Server Banner" src="https://discordapp.com/api/guilds/1087452688956006471/widget.png?style=banner2" />](https://discord.gg/rPvFW4jQTJ)

You can also contact the developer of the mod (HJfod) directly through Discord or Twitter/X.

 * Discord: `@hjfod`
 * Twitter: [`hjfod`](https://twitter.com/hjfod)

## :euro: Support

BetterEdit's development is supported via [donations on my Ko-fi](https://ko-fi.com/hjfod)!

## :balance_scale: Licensing

**BetterEdit is licensed under the [LGPLv3](https://www.gnu.org/licenses/lgpl-3.0.en.html) license.**

This means **you cannot create closed-source versions of BetterEdit**. You, however, *can* create separate closed-source mods that depend on BetterEdit.

This is to ensure that nobody piggybacks off the hundreds of hours of work spent by myself and other developers, without providing proper credit in the form of the free version! :blush:
