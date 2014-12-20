/*
	Changes for Maki engine_t:
	- Lots of stuff stripped out
	- functions converted from using FILE* to using iostreams

	Original code header and license info included below:
*/


/*********************************************************************\

MODULE NAME:    b64.c

AUTHOR:         Bob Trower 08/04/01

PROJECT:        Crypt Data Packaging

COPYRIGHT:      Copyright (c) Trantor Standard Systems Inc., 2001

NOTES:          This source code may be used as you wish, subject to
                the MIT license.  See the LICENCE section below.

                Canonical source should be at:
                    http://base64.sourceforge.net

DESCRIPTION:
                This little utility implements the base64
                Content-Transfer-Encoding standard described in
                RFC1113 (http://www.faqs.org/rfcs/rfc1113.html).

                This is the coding scheme used by MIME to allow
                binary data to be transferred by SMTP mail.

                Groups of 3 bytes from a binary stream are coded as
                groups of 4 bytes in a text stream.

                The input stream is 'padded' with zeros to create
                an input that is an even multiple of 3.

                A special character ('=') is used to denote padding so
                that the stream can be decoded back to its exact size.

                Encoded output is formatted in lines which should
                be a maximum of 72 characters to conform to the
                specification.  This program defaults to 72 characters,
                but will allow more or less through the use of a
                switch.  The program enforces a minimum line size
                of 4 characters.

                Example encoding:

                The stream 'ABCD' is 32 bits long.  It is mapped as
                follows:

                ABCD

                 A (65)     B (66)     C (67)     D (68)   (None) (None)
                01000001   01000010   01000011   01000100

                16 (Q)  20 (U)  9 (J)   3 (D)    17 (R) 0 (A)  NA (=) NA (=)
                010000  010100  001001  000011   010001 000000 000000 000000


                QUJDRA==

                Decoding is the process in reverse.  A 'decode' lookup
                table has been created to avoid string scans.

DESIGN GOALS:	Specifically:
		Code is a stand-alone utility to perform base64 
		encoding/decoding. It should be genuinely useful 
		when the need arises and it meets a need that is 
		likely to occur for some users.  
		Code acts as sample code to show the author's 
		design and coding style.  

		Generally: 
		This program is designed to survive:
		Everything you need is in a single source file.
		It compiles cleanly using a vanilla ANSI C compiler.
		It does its job correctly with a minimum of fuss.  
		The code is not overly clever, not overly simplistic 
		and not overly verbose. 
		Access is 'cut and paste' from a web page.  
		Terms of use are reasonable.  

VALIDATION:     Non-trivial code is never without errors.  This
                file likely has some problems, since it has only
                been tested by the author.  It is expected with most
                source code that there is a period of 'burn-in' when
                problems are identified and corrected.  That being
                said, it is possible to have 'reasonably correct'
                code by following a regime of unit test that covers
                the most likely cases and regression testing prior
                to release.  This has been done with this code and
                it has a good probability of performing as expected.

                Unit Test Cases:

                case 0:empty file:
                    CASE0.DAT  ->  ->
                    (zero length target file created
                    on both encode and decode.)

                case 1:One input character:
                    CASE1.DAT A -> QQ== -> A

                case 2:Two input characters:
                    CASE2.DAT AB -> QUI= -> AB

                case 3:Three input characters:
                    CASE3.DAT ABC -> QUJD -> ABC

                case 4:Four input characters:
                    case4.dat ABCD -> QUJDRA== -> ABCD

                case 5:All chars from 0 to ff, linesize set to 50:

                    AAECAwQFBgcICQoLDA0ODxAREhMUFRYXGBkaGxwdHh8gISIj
                    JCUmJygpKissLS4vMDEyMzQ1Njc4OTo7PD0+P0BBQkNERUZH
                    SElKS0xNTk9QUVJTVFVWV1hZWltcXV5fYGFiY2RlZmdoaWpr
                    bG1ub3BxcnN0dXZ3eHl6e3x9fn+AgYKDhIWGh4iJiouMjY6P
                    kJGSk5SVlpeYmZqbnJ2en6ChoqOkpaanqKmqq6ytrq+wsbKz
                    tLW2t7i5uru8vb6/wMHCw8TFxsfIycrLzM3Oz9DR0tPU1dbX
                    2Nna29zd3t/g4eLj5OXm5+jp6uvs7e7v8PHy8/T19vf4+fr7
                    /P3+/w==

                case 6:Mime Block from e-mail:
                    (Data same as test case 5)

                case 7: Large files:
                    Tested 28 MB file in/out.

                case 8: Random Binary Integrity:
                    This binary program (b64.exe) was encoded to base64,
                    back to binary and then executed.

                case 9 Stress:
                    All files in a working directory encoded/decoded
                    and compared with file comparison utility to
                    ensure that multiple runs do not cause problems
                    such as exhausting file handles, tmp storage, etc.

                -------------

                Syntax, operation and failure:
                    All options/switches tested.  Performs as
                    expected.

                case 10:
                    No Args -- Shows Usage Screen
                    Return Code 1 (Invalid Syntax)
                case 11:
                    One Arg (invalid) -- Shows Usage Screen
                    Return Code 1 (Invalid Syntax)
                case 12:
                    One Arg Help (-?) -- Shows detailed Usage Screen.
                    Return Code 0 (Success -- help request is valid).
                case 13:
                    One Arg Help (-h) -- Shows detailed Usage Screen.
                    Return Code 0 (Success -- help request is valid).
                case 14:
                    One Arg (valid) -- Uses stdin/stdout (filter)
                    Return Code 0 (Sucess)
                case 15:
                    Two Args (invalid file) -- shows system error.
                    Return Code 2 (File error)
                case 16:
                    encode non-existent file -- shows system error.
                    Return Code 2 (File error)
                case 17:
                    Out of disk space -- shows system error.
                    Return Code 3 (File I/O error)
                case 18:
                    Too many args -- shows system error.
                    Return Code 1 (Invalid Syntax)

                -------------

                Compile/Regression test:
                    gcc compiled binary under Cygwin
                    Microsoft Visual Studio under Windows 2000
                    Microsoft Version 6.0 C under Windows 2000

DEPENDENCIES:   None

LICENCE:        Copyright (c) 2001 Bob Trower, Trantor Standard Systems Inc.

                Permission is hereby granted, free of charge, to any person
                obtaining a copy of this software and associated
                documentation files (the "Software"), to deal in the
                Software without restriction, including without limitation
                the rights to use, copy, modify, merge, publish, distribute,
                sublicense, and/or sell copies of the Software, and to
                permit persons to whom the Software is furnished to do so,
                subject to the following conditions:

                The above copyright notice and this permission notice shall
                be included in all copies or substantial portions of the
                Software.

                THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY
                KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
                WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
                PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS
                OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
                OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
                OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
                SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

VERSION HISTORY:
                Bob Trower 08/04/01 -- Create Version 0.00.00B
                Bob Trower 08/17/01 -- Correct documentation, messages.
                                    -- Correct help for linesize syntax.
                                    -- Force error on too many arguments.
                Bob Trower 08/19/01 -- add sourceforge.net reference to
                                       help screen prior to release.
                Bob Trower 10/22/04 -- Cosmetics for package/release.
                Bob Trower 02/28/08 -- More Cosmetics for package/release.
                Bob Trower 02/14/11 -- Cast altered to fix warning in VS6.

\******************************************************************* */

#pragma once
#include "core/core_stdafx.h"

namespace maki
{
	namespace core
	{
	
		namespace base64
		{
	
			// translation Table as described in RFC1113
			static const char cb64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

			// translation Table to decode (created by author)
			static const char cd64[] = "|$$$}rstuvwxyz{$$$$$$$>?@ABCDEFGHIJKLMNOPQRSTUVW$$$$$$XYZ[\\]^_`abcdefghijklmnopq";


			/*
			** encode_block
			**
			** encode 3 8-bit binary bytes as 4 '6-bit' characters
			*/
			inline void encode_block(unsigned char *in, unsigned char *out, int len)
			{
				out[0] = (unsigned char)cb64[ (int)(in[0] >> 2) ];
				out[1] = (unsigned char)cb64[ (int)(((in[0] & 0x03) << 4) | ((in[1] & 0xf0) >> 4)) ];
				out[2] = (unsigned char)(len > 1 ? cb64[ (int)(((in[1] & 0x0f) << 2) | ((in[2] & 0xc0) >> 6)) ] : '=');
				out[3] = (unsigned char)(len > 2 ? cb64[ (int)(in[2] & 0x3f) ] : '=');
			}

			/*
			** decode_block
			**
			** decode 4 '6-bit' characters into 3 8-bit binary bytes
			*/
			inline void decode_block(unsigned char *in, unsigned char *out)
			{   
				out[0] = (unsigned char)(in[0] << 2 | in[1] >> 4);
				out[1] = (unsigned char)(in[1] << 4 | in[2] >> 2);
				out[2] = (unsigned char)(((in[2] << 6) & 0xc0) | in[3]);
			}



			bool decode(std::istream &src, std::ostream &dst)
			{
				int retcode = 0;
				unsigned char in[4];
				unsigned char out[3];
				int v;
				int i, len;

				*in = (unsigned char) 0;
				*out = (unsigned char) 0;
				while(!src.eof()) {
					for(len = 0, i = 0; i < 4 && !src.eof(); i++) {
						v = 0;
						while(!src.eof() && v == 0) {
							v = src.get();
							if(v != EOF) {
								v = ((v < 43 || v > 122) ? 0 : (int) cd64[ v - 43 ]);
								if(v != 0) {
									v = ((v == (int)'$') ? 0 : v - 61);
								}
							}
						}
						if(!src.eof()) {
							len++;
							if( v != 0 ) {
								in[ i ] = (unsigned char) (v - 1);
							}
						} else {
							in[i] = (unsigned char) 0;
						}
					}
					if(len > 0) {
						decode_block(in, out);
						for(i = 0; i < len - 1; i++) {
							dst.put(out[i]);
							if(dst.bad()){
								return false;
							}
						}
					}
				}
				return true;
			}

			bool encode(std::istream &src, std::ostream &dst, int linesize = 1024)
			{
				unsigned char in[3];
				unsigned char out[4];
				int i, len, blocksout = 0;
				int retcode = 0;

				*in = (unsigned char)0;
				*out = (unsigned char)0;
				while(!src.eof()) {
					len = 0;
					for(i = 0; i < 3; i++) {
						in[i] = src.get();

						if(!src.eof()) {
							len++;
						}
						else {
							in[i] = (unsigned char) 0;
						}
					}
					if(len > 0) {
						encode_block(in, out, len);
						for(i = 0; i < 4; i++) {
							dst.put(out[i]);
							if(dst.bad()) {
								return false;
							}
						}
						blocksout++;
					}
					if(blocksout >= (linesize/4)) {
						if( blocksout > 0 ) {
							dst.put('\n');
						}
						blocksout = 0;
					}
				}
				return true;
			}

		} // namespace base64

	} // namespace core

} // namespace maki
