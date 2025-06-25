import hashlib

# 1. Read the original firmware
with open("../build/SecureBoot_F407.bin", "rb") as target:
    firmware = target.read()

# 2. Calculate how much to hash (everything except last 32 bytes)
HASH_SIZE = 32  # SHA-256 creates 32-bytes hashes
hash_region = firmware[:-HASH_SIZE]

# 3. Compute SHA-256 hash of the main content
sha256 = hashlib.sha256()
sha256.update(hash_region)
digest = sha256.digest()  # get the 32-byte hash

# 4. Create patched firmware (original content + new hash)
patch_firmware = hash_region + digest

# Verify sizes match (original file size should be maintained)
assert len(patch_firmware) == len(firmware)

# 5. Save the patched firmware
with open("../build/SecureBoot_F407_hashed.bin", "wb") as target:
    target.write(patch_firmware)

print(f"SHA-256 hash written to 0x{0x0807FFE0:08X}")
