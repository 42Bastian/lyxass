#ifndef _OUTPUT_H_
#define _OUTPUT_H_ 1

void writeByte(char );
void writeBytes(char *, int);
void writeSameBytes(char , int );
void writeWordLittle(short );
void writeWordBig(short );
void writeLongLittle(int32_t );
void writeLongBig(int32_t );
void writePhraseLittle(int64_t );
void writePhraseBig(int64_t );
void writeRelocByte(char c, int reloc);
void writeConstByte(char c);
#endif /*_OUTPUT_H_*/
