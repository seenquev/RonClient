/* -------------------------------------------------------------------------- */
/* ------------- RonClient --- Oficial client for RonOTS servers ------------ */
/* -------------------------------------------------------------------------- */

#ifndef __RSA_H_
#define __RSA_H_

#include "gmp.h"
#include "mthread.h"

class RSA{
public:
	RSA();
	~RSA();
	void setKey(const char* p, const char* q, const char* d);
	bool setKey(const std::string& file);
	bool encrypt(char* msg, int32_t size);
	bool decrypt(char* msg, int32_t size);
	
	int32_t getKeySize();
	void getPublicKey(char* buffer);
	
public:
	
	MUTEX lockRsa;
	
	bool m_keySet;
	
	//use only GMP
	mpz_t m_p, m_q, m_u, m_d, m_dp, m_dq, m_mod;
};

#endif //__RSA_H_
