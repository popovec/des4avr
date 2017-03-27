#ifndef _DES_H_
#define _DES_H_

#define ENCRYPTION_MODE 1
#define DECRYPTION_MODE 0

void des_run (uint8_t * data,  uint8_t *key, uint8_t mode);

#endif
