/* Algorithm "xor" from p. 4 of Marsaglia, "Xorshift RNGs" */
unsigned int random32(unsigned int number)
{
	unsigned int x = number;
	x ^= x << 13;
	x ^= x >> 17;
	x ^= x << 5;
	return number = x;
}