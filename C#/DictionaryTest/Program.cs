using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace DictionaryTest
{
    class Program
    {
#if DEBUG
        static readonly long kIterations = 10000;
#else
        static readonly long kIterations = 3000000;
#endif

        static String[] header_fields = {
            // Request
            "Accept",
            "Accept-Charset",
            "Accept-Encoding",
            "Accept-Language",
            "Authorization",
            "Cache-Control",
            "Connection",
            "Cookie",
            "Content-Length",
            "Content-MD5",
            "Content-Type",
            "Date",
            "DNT",
            "From",
            "Front-End-Https",
            "Host",
            "If-Match",
            "If-Modified-Since",
            "If-None-Match",
            "If-Range",
            "If-Unmodified-Since",
            "Max-Forwards",
            "Pragma",
            "Proxy-Authorization",
            "Range",
            "Referer",
            "User-Agent",
            "Upgrade",
            "Via",
            "Warning",
            "X-ATT-DeviceId",
            "X-Content-Type-Options",
            "X-Forwarded-For",
            "X-Forwarded-Proto",
            "X-Powered-By",
            "X-Requested-With",
            "X-XSS-Protection",

            // Response
            "Access-Control-Allow-Origin",
            "Accept-Ranges",
            "Age",
            "Allow",
            //"Cache-Control",
            //"Connection",
            "Content-Encoding",
            "Content-Language",
            //"Content-Length",
            "Content-Disposition",
            //"Content-MD5",
            "Content-Range",
            //"Content-Type",
            "Date",
            "ETag",
            "Expires",
            "Last-Modified",
            "Link",
            "Location",
            "P3P",
            "Proxy-Authenticate",
            "Refresh",
            "Retry-After",
            "Server",
            "Set-Cookie",
            "Strict-Transport-Security",
            "Trailer",
            "Transfer-Encoding",
            "Vary",
            "Via",
            "WWW-Authenticate",
            //"X-Content-Type-Options",
            //"X-Powered-By",
            //"X-XSS-Protection",

            "Last"
        };

        static void hashtable_find_benchmark_impl()
        {
            long kHeaderFieldSize = header_fields.Length;
            long kRepeatTimes = (kIterations / kHeaderFieldSize);

            string[] field_str = new string[kHeaderFieldSize];
            string[] index_str = new string[kHeaderFieldSize];
            for (long i = 0; i < kHeaderFieldSize; ++i)
            {
                field_str[i] = header_fields[i];
                index_str[i] = string.Format("{0}", i);
            }

            {
                Dictionary<string, string> dict = new Dictionary<string, string>();
                for (long i = 0; i < kHeaderFieldSize; ++i)
                {
                    if (!dict.ContainsKey(field_str[i]))
                        dict.Add(field_str[i], index_str[i]);
                }

                long checksum = 0;
                Stopwatch sw = new Stopwatch();

                sw.Restart();
                for (long i = 0; i < kRepeatTimes; ++i)
                {
                    for (long j = 0; j < kHeaderFieldSize; ++j)
                    {
                        bool hasKey = dict.ContainsKey(field_str[j]);
                        if (hasKey)
                        {
                            checksum++;
                        }
                    }
                }
                sw.Stop();

                Console.Write("-------------------------------------------------------------------------\n");
                Console.Write(" {0,-28}  ", "Dictionary<string, string>");
                Console.Write("sum = {0,-10:g}  time: {1,8:f} ms\n", checksum, sw.Elapsed.TotalMilliseconds);
                Console.Write("-------------------------------------------------------------------------\n");
                Console.Write("\n");
            }
        }

        static void dictionary_find_benchmark()
        {
            Console.Write("-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n");
            Console.Write("  dictionary_find_benchmark()\n");
            Console.Write("-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n");
            Console.Write("\n");

            hashtable_find_benchmark_impl();
        }

        static void hashtable_insert_benchmark_impl()
        {
            long kHeaderFieldSize = header_fields.Length;
#if DEBUG
            long kRepeatTimes = 100;
#else
            long kRepeatTimes = (kIterations / kHeaderFieldSize);
#endif

            string[] field_str = new string[kHeaderFieldSize];
            string[] index_str = new string[kHeaderFieldSize];
            for (long i = 0; i < kHeaderFieldSize; ++i)
            {
                field_str[i] = header_fields[i];
                index_str[i] = string.Format("{0}", i);
            }

            {
                long checksum = 0;
                double totalTime = 0;
                Stopwatch sw = new Stopwatch();
                
                for (long i = 0; i < kRepeatTimes; ++i)
                {
                    Dictionary<string, string> dict = new Dictionary<string, string>();

                    sw.Restart();
                    for (long j = 0; j < kHeaderFieldSize; ++j)
                    {
                        if (!dict.ContainsKey(field_str[j]))
                            dict.Add(field_str[j], index_str[j]);
                    }
                    sw.Stop();

                    checksum += dict.Count();
                    totalTime += sw.Elapsed.TotalMilliseconds;
                }

                Console.Write("-------------------------------------------------------------------------\n");
                Console.Write(" {0,-28}  ", "Dictionary<string, string>");
                Console.Write("sum = {0,-10:g}  time: {1,8:f} ms\n", checksum, totalTime);
                Console.Write("-------------------------------------------------------------------------\n");
                Console.Write("\n");
            }
        }

        static void dictionary_insert_benchmark()
        {
            Console.Write("-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n");
            Console.Write("  dictionary_insert_benchmark()\n");
            Console.Write("-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n");
            Console.Write("\n");

            hashtable_insert_benchmark_impl();
        }

        static void dictionary_erase_benchmark()
        {
            Console.Write("-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n");
            Console.Write("  dictionary_erase_benchmark()\n");
            Console.Write("-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n");
            Console.Write("\n");

            hashtable_erase_benchmark_impl();
        }

        static void dictionary_insert_erase_benchmark()
        {
            //
        }

        static void dictionary_rehash_benchmark()
        {
            //
        }

        static void dictionary_rehash2_benchmark()
        {
            //
        }

        static void dictionary_benchmark()
        {
            dictionary_find_benchmark();

            dictionary_insert_benchmark();
            dictionary_erase_benchmark();
            dictionary_insert_erase_benchmark();

            dictionary_rehash_benchmark();
            dictionary_rehash2_benchmark();
        }

        static void Main(string[] args)
        {
            dictionary_benchmark();

            //Console.Write("\n");
            Console.Write("Press any key ...");
            Console.ReadKey();
        }
    }
}
