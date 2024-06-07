; This pads the kernel with 10 kb of zeros, so that the amount of sectors loaded doesn't have to be edited each time.
times 10000-($-$$) db '0'
