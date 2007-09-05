int main()
{
	unsigned int i;

	for (i=0;i<100001;i++) {
		if ((i%20000)==0) { printf("%d\n", i); }
	}
	return 0;
}
