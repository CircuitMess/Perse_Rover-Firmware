idf_component_register(SRCS "hooks.c" PRIV_REQUIRES hal)

# The bootloader hooks are weak symbols, so the linker would drop this object file unless
# something forces it in. Defining bootloader_hooks_include() in hooks.c does that.
