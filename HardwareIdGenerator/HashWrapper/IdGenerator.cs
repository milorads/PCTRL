using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using wifiConnectionTest;

namespace IdGenerator
{
    public static class IdGenerator
    {
        private static string GenerateShortFilenameId()
        {
            return ShortId.GetBase36(5);
        }

        private static string GenerateId()
        {
            return Guid.NewGuid().ToString();
        }

        private static string GetHashedId(string id)
        {
            return HashMd5.GetHashedId(id);
        }

        private static string GenerateQrCode(string id, string shortFileNameId)
        {
            return QrCodeGenerator.GenerateQrCode(id, shortFileNameId);
        }

        public static string GetId(out string id, out string hashedId)
        {
            var shortFileName = GenerateShortFilenameId();
            id = GenerateId();
            hashedId = GetHashedId(id);
            return GenerateQrCode(hashedId, shortFileName);
        }
    }
}
