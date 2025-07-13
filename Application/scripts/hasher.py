import hashlib
import subprocess
from cryptography.hazmat.primitives.asymmetric import utils

# Configuration
HASH_SIZE = 32  # SHA-256 produces 32-byte hashes
SIGNATURE_SIZE = 64  # ECDSA P-256 signature is 64 bytes (r||s format)

# Run objcopy to generate full bin with gaps preserved
subprocess.run(
    [
        "arm-none-eabi-objcopy",
        "-O",
        "binary",
        "../build/SecureBoot_F407.elf",
        "../build/SecureBoot_F407_full.bin",
    ],
    check=True,
)

# Read the original firmware
with open("../build/SecureBoot_F407_full.bin", "rb") as target:
    firmware = target.read()

# Calculate hash region (everything except reserved space for hash + signature)
hash_region = firmware[:-(HASH_SIZE + SIGNATURE_SIZE)]

# Compute SHA-256 hash of the main content
sha256 = hashlib.sha256()
sha256.update(hash_region)
digest = sha256.digest()

# Save hash to temporary file for signing
with open("../build/firmware.hash", "wb") as f:
    f.write(digest)

# Sign the hash using OpenSSL (ECDSA)
subprocess.run(
    [
        "openssl", "dgst", "-sha256",
        "-sign", "../ec_private.pem",
        "-out", "../build/firmware.sig",
        "../build/firmware.hash"
    ],
    check=True,
)

# Read and process the signature
with open("../build/firmware.sig", "rb") as f:
    signature = f.read()

# Convert DER to raw r||s
r, s = utils.decode_dss_signature(signature)

# Convert integers to bytes, pad to 32 bytes
r_bytes = r.to_bytes(32, byteorder='big')
s_bytes = s.to_bytes(32, byteorder='big')

# Concatenate to final signature
raw_signature = r_bytes + s_bytes
signature = raw_signature

print("Python raw signature (r||s):", raw_signature.hex())

# Final check
assert len(signature) == 64, "Signature must be exactly 64 bytes after conversion!"

# Create final firmware image (original + hash + signature)
patched_firmware = hash_region + digest + signature

# Verify final size matches expectations
assert len(patched_firmware) == len(firmware), "Final firmware size mismatch"

# Save the signed firmware
with open("../build/SecureBoot_F407_signed.bin", "wb") as target:
    target.write(patched_firmware)

# Print information
hash_offset = len(hash_region)
sig_offset = hash_offset + HASH_SIZE
print("SHA-256 hash and ECDSA signature embedded successfully.")
print(f"Hash offset: 0x{hash_offset:08X} ({hash_offset} bytes)")
print(f"Signature offset: 0x{sig_offset:08X} ({sig_offset} bytes)")
print(f"Total reserved area: {HASH_SIZE + SIGNATURE_SIZE} bytes")

