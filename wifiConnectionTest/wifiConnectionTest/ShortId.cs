using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace wifiConnectionTest
{
    public static class ShortId
    {
        //All the characters that we can use in our ID generator
        private static char[] _base62chars =
            "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"
                .ToCharArray();
        //Random class instance
        private static Random _random = new Random();
        /// <summary>
        /// Generate id that uses 62 characters
        /// </summary>
        /// <param name="length">Length of id</param>
        /// <returns></returns>
        public static string GetBase62(int length)
        {
            var sb = new StringBuilder(length);

            for (int i = 0; i < length; i++)
                sb.Append(_base62chars[_random.Next(62)]);

            return sb.ToString();
        }
        /// <summary>
        /// Generate id that uses 36 characters. e.g. filenames don't recognize upper/lower case
        /// </summary>
        /// <param name="length">Length of id</param>
        /// <returns></returns>
        public static string GetBase36(int length)
        {
            var sb = new StringBuilder(length);

            for (int i = 0; i < length; i++)
                sb.Append(_base62chars[_random.Next(36)]);

            return sb.ToString();
        }
    }
}