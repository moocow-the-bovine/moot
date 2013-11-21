/*!re2c
	URI_ESCAPE 		= "%" XDIGIT{2} ;

	URI_GEN_DELIMS		= [:/?#\[\]@] ;
	URI_SUB_DELIMS		= [!&'\(\)\*\+,;=\$] ;
	URI_RESERVED		= URI_GEN_DELIMS | URI_SUB_DELIMS ;
	URI_FREE		= [A-Za-z0-9\x80-\xff_\-\.~] ;
	URI_SUBFREE		= [A-Za-z0-9\x80-\xff_\-\.~!&'\(\)\*\+,;=\$%] ;
	URI_USERCHAR		= [A-Za-z0-9\x80-\xff_\-\.~!&'\(\)\*\+,;=\$%:] ;
	URI_HOSTCHAR__ext	= [A-Za-z0-9\x80-\xff_\-\.~!&'\(\)\*\+,;=\$%@:]  ;
	URI_HOSTCHAR		= [A-Za-z0-9\x80-\xff_\-~!&'\(\)\*\+,;=\$%@]  ;
	URI_PATHCHAR__rfc3986	= ( URI_SUBFREE | [:@] );
	URI_PATHCHAR		= [A-Za-z0-9\x80-\xff_\-\.~!&'\(\)\*\+,;=\$%:@<>{}\[\]`];
	URI_PATHCHAR__ext	= [A-Za-z0-9\x80-\xff_\-\.~!&'\(\)\*\+,;=\$%:@<>{}\[\]`/];
	URI_QUERYCHAR		= [A-Za-z0-9\x80-\xff_\-\.~!&'\(\)\*\+,;=\$%:@<>{}\[\]`/?];
	URI_FRAGMENTCHAR	= [A-Za-z0-9\x80-\xff_\-\.~!&'\(\)\*\+,;=\$%:@<>{}\[\]`/?#];

	URI_SCHEME		= [A-Za-z][A-Za-z0-9\+\-\.]* ;

	URI_IP_FUTURE		= "v" (XDIGIT)+ "." ( URI_FREE | URI_SUB_DELIMS | ":" )+ ;
	URI_IP_LITERAL		= "[" (IPv6 | URI_IP_FUTURE) "]" ;
	URI_HOSTNAME            = ( URI_SUBFREE )+ ;
	URI_HOSTNAME__dots	= URI_HOSTCHAR{2,} ("." URI_HOSTCHAR{2,})+ ;
	URI_PORT		= [0-9]* ;
	URI_HOST_ADDR		= URI_IP_LITERAL | IPv4;
	URI_HOST        	= URI_HOST_ADDR | URI_HOSTNAME       ;
	URI_HOST__dots        	= URI_HOST_ADDR | URI_HOSTNAME__dots ;
	URI_USERINFO		= URI_USERCHAR* ;
	URI_AUTHORITY           = (URI_USERINFO "@")? URI_HOST       (":" URI_PORT)? ;
	URI_AUTHORITY__udots	= (URI_USERINFO "@")? URI_HOST__dots (":" URI_PORT)? ;
	URI_AUTHORITY__dots	= 	              URI_HOST__dots (":" URI_PORT)? ;
	URI_AUTHORITY__addr	= (URI_USERINFO "@")? URI_HOST_ADDR  (":" URI_PORT)? ;
	URI_AUTHORITY__safe1    = URI_USERINFO ("@"|"www.") URI_HOSTCHAR{2,} ("." URI_HOSTCHAR{2,})* (":" URI_PORT)? ;
	URI_AUTHORITY__safe2    = URI_HOSTCHAR+ ("." URI_HOSTCHAR+)* ("." [a-zA-Z]{2,3}) (":" URI_PORT)? ;

	URI_AUTHORITY__safe     = URI_AUTHORITY__addr | URI_AUTHORITY__safe1 | URI_AUTHORITY__safe2 | "#" ;

	URI_SEGMENT		= URI_PATHCHAR* ;
	URI_PATH_NONEMPTY	= URI_PATHCHAR__ext+ ;
	URI_PATH 		= ("/" URI_SEGMENT)* ;
	URI_PATH__abs		= "/" URI_PATHCHAR__ext+ ;

	URI_QUERY		= URI_QUERYCHAR* ;
	URI_FRAGMENT		= URI_FRAGMENTCHAR* ;

	URI__rfc3986	        = URI_SCHEME ":" "//"        URI_AUTHORITY        URI_PATH       ("?" URI_QUERY)?  ("#" URI_FRAGMENT)? ;
	URI__schemed	        = URI_SCHEME ":" "/"{1,3}    URI_FRAGMENTCHAR{2,} ;
	URI__link_dots		=                            URI_AUTHORITY__udots URI_FRAGMENTCHAR* ;
	URI__link_safe0		=                            URI_AUTHORITY__safe  URI_FRAGMENTCHAR* ;

	URI__link_safe1		= URI_USERINFO ("@"|"www.") URI_FRAGMENTCHAR* ;
	URI__link_safe2		= (URI_USERINFO "@")? URI_AUTHORITY__addr ([/?#] URI_FRAGMENTCHAR*)? ;
	URI__link_safe3		= URI_HOSTCHAR__ext+ ("." ("at"|"ch"|"de"|"com"|"net"|"org")) ([/?#] URI_FRAGMENTCHAR*)? ;

	URI__link_host_www	= ("@"|"www.") URI_HOSTCHAR__ext+ ;
	URI__link_host_addr	= "@"? URI_AUTHORITY__addr ;
	URI__link_host_tld	= URI_HOSTCHAR__ext+ "." ("at"|"ch"|"de"|"com"|"net"|"org") (":" URI_PORT)? ;
	URI__link_host		= URI__link_host_www | URI__link_host_addr | URI__link_host_tld ;
	URI__link_suffix	= [/?#] URI_FRAGMENTCHAR+ ;

	URI__link_at		= URI_USERINFO "@" URI_FRAGMENTCHAR+ ;
	URI__link_hash		= "#" ALPHA (URI_FRAGMENTCHAR*) (ALPHA|DIGIT) ;
	URI__link_www		= URI_USERINFO URI__link_host_www URI__link_suffix? ;
	URI__link_safe		= URI_USERINFO URI__link_host URI__link_suffix? ;
				  
	LINK			= URI__schemed | URI__link_at | URI__link_hash ;
*/

        //-- old link rules (pre-2013-11-18; ultimate target: LINK2) -- ought to work again with NUL sentinel
        /*!re2c
        MAILPART                = [A-Za-z0-9\._\-]+;
        URI1a                   = (MAILPART "@")?([a-z] MAILPART ":" ("/"{1,3}|[a-z0-9%]));
        URI1b                   = ([^ \x00\n\t\r\(\)<>]+|"("([^ \x00\t\n\r\(\)<>]+|("("[^ \x00\n\t\r\(\)<>]+")"))*")")*;
        URI1c                   = ("("([^ \x00\t\n\r\(\)<>]+|("("[^ \x00\n\t\r\(\)<>]+")"))*")"|[^ \x00\n\t\r`!\(\)\[\]{};:'\"\.,<>?«»“”‘’]);

        URI2a                   = (MAILPART "@")?[a-z0-9\.\-]+[\.][a-zA-z]{2,6};
        URI2b                   = ([^ \x00\t\n\r\(\)<>]*[^ \x00\n\t\r`!\(\)\[\]{};:'\"\.,<>?«»“”‘’]|"("([^ \x00\t\n\r\(\)<>]+|("("[^ \x00\n\t\r\(\)<>]+")"))*")")*;
        
        URI1                    = URI1a URI1b URI1c;
        URI2                    = URI2a URI2b;

        LINK2                   = IP|URI1|URI2;
*/


        //-- old link rules, rewritten Thu, 21 Nov 2013 10:09:01 +0100
        HOSTCHAR	        = [A-Za-z0-9\._\-~] ;
        HOSTCHAR_LOWER	        = [a-z0-9\._\-~] ;
        URI_HOST                = (HOSTCHAR* "@")?([a-z] HOSTCHAR+ ":" ("/"{1,3}|[a-z0-9%]));

	PATHCHAR		= [^ \x00\t\n\r\(\)<>];
        PATHCHAR_SAFE__old	= [^ \x00\n\t\r`!\(\)\[\]{};:'\"\.,<>?«»“”‘’] ;
        PATHCHAR_SAFE		= [^ \x00\n\t\r`!\(\)\[\]{};:'\"\.,<>?«»“”‘’] ;
	URI_PATH_DEPTH0		= PATHCHAR* PATHCHAR_SAFE ;
	URI_PATH_DEPTH1		= "(" URI_PATH_DEPTH0 ")" ;
	URI_PATH_DEPTH2		= "(" (URI_PATH_DEPTH0 | URI_PATH_DEPTH1)* ")" ;
	URI_PATH		= (URI_PATH_DEPTH0 | URI_PATH_DEPTH1 | URI_PATH_DEPTH2)* ;

	URI2_TLD		= [a-z]{2} | "biz"|"com"|"edu"|"gov"|"info"|"mobi"|"net"|"org" ;
	URI2_HOST		= (HOSTCHAR* "@")? HOSTCHAR_LOWER+ "." URI2_TLD ;

	URI1			= URI_HOST  URI_PATH ;
	URI2			= URI2_HOST URI_PATH ;

        LINK1__old              = ([A-Za-z]+"://")?([A-Za-z0-9]+[\.])+[A-Za-z0-9]{2,6}[/A-Za-z0-9?:~\.]*;
        LINK2			= IP | URI1 | URI2 ;
	 */
*/