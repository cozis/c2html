/* Takes as input a string of C code [str] of length
 * [len] and returns the same C code but annotated
 * with HTML tags. The returned string must be freed
 * using [free]. The class names of each tag are
 * prefixed with the value provided through [prefix].
 * If [prefix] is NULL, then no prefix is used.
 *
 * If the function fails, NULL is returned and a
 * human-readable description of the error is
 * returned through the output parameter [error].
 * The returned error string doesn't need to be
 * deallocated. If you're not interested in the
 * error description, you can just provide NULL.
 *
 * The [str] string isn't assumed to be zero-terminated
 * and if it's NULL, an empty string is assumed.
 *
 * If [len] is negative, then [str] is assumed 
 * to be zero-terminated and it's length is calculated
 * using [strlen].
 */
char *c2html(const char *str, long len, 
             const char *prefix, const char **error);
