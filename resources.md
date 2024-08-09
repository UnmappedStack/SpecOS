# OS development resources

Operating system development is pretty hard, and I'm probably a below-average developer - so how did I work out how to program SpecOS (despite it still being in very early stages)?

I read the f\*cking manual. Here's some resources I used:

- When writing the VGA driver, keyboard driver, PIO hard disk driver, and basically everything else, I made use of the [OSDev wiki](https://wiki.osdev.org). Note that this is definitely *not* a tutorial. It just provides some useful information. 
- For implementing the FAT32 file system (which I'm still in the process of), I just followed the [official Microsoft documentation](https://download.microsoft.com/download/1/6/1/161ba512-40e2-4cc9-843a-923143f3456c/fatgen103.doc). Again, this contains information and is definitely not a tutorial.

> **NOTE:**
>
> If you would like less limits on your file system, but a potentially more challenging file system to implement, I *highly* recommend exFAT. Check out the official docs [here](https://learn.microsoft.com/en-us/windows/win32/fileio/exfat-specification).


## Some of my personal opinions about OS development

You came here for the resources, stayed for my opinions. Well, here's a short list.

Note that I'm still somewhat a beginner to OS development myself, so take what I say with a grain of salt.

- Don't write your own bootloader. Seriously, just use GRUB. Your main goal should be a decent kernel at the beginning. I was previously using a custom bootloader, but now that I'm using a FAT32 file system, I've switched over to GRUB.
- Don't use a non-standard language. C, C++, and Rust are fine (even Rust I'm a little iffy about). The issue with languages like Perl are that there's far less people in the OS dev community who will know how to help find issues in your code - and having other people look at your code *really* helps.
- A good kernel is more important than a good userspace. Userspace definitely matters a lot, but it can only be decent with a great kernel. Write your kernel, perfect it, then start working on a basic userspace.
- Bad code > no code. Make it work, then when you start thinking about getting other people to join, improve it and refactor. Still though, don't write crappy code from the start, just don't worry about it too much in the beginning stages. Maybe I'm biased though, because my code is messy as all hell.
- Get out of your comfort zone! OS development is a really hard project, so try some new technology to help you to make it that little bit easier. For example, soon before I started OS dev, I started using Vim. My programming has been much faster since.
- **Don't take advice from some random guy on the internet who's a beginner himself and has told you so many times - find somebody who knows what they're talking about.** Yeah, I'm saying don't listen to anything I say.

## Some cool projects

I didn't invent operating systems, obviously. I'm inspired by some other operating systems myself (all hobby ones), which are what make me want to do this project. Check them out, and shoot them a star :)

- [Banan-os](https://git.bananymous.com/Bananymous/banan-os) is a *very* cool project, which from what I can tell, has a fairly complete kernel, and is now moving on to a GUI. This thing even has a network stack! Most importantly, it runs DOOM. This is probably my goal at some point.
- [SkiftOS](http://github.com/skift-org/skift) uses a micro-kernel, and is very graphics-oriented, with a complete user interface. After trying it out, I could only be impressed - I'd love to see it a little faster without needing kvm, but it's a really great project.
- [SerenityOS](https://github.com/SerenityOS/serenity) isn't just an operating system. It's a whole suite of apps, ported from other operating systems. Andreas Kling, the developer, even wrote for it an entire web browser. This is *actually genuinely usable* as a daily driver, by the seems of it. Maybe it's time to replace my Arch setup?

The above are really amazing. Please try them out, star their repos, because they are far, far more than I could ever achieve (and in the nicest way possible, probably more than you could achieve).
