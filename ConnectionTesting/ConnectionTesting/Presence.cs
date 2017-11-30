using System;
using System.Collections.Generic;
using System.Text;

namespace wifiConnectionTest
{
    class Presence
    {
        public Presence(string ime, string prezime, string id, DateTime ulaz, DateTime izlaz)
        {
            Ime = ime;
            Prezime = prezime;
            Id = id;
            Ulaz = ulaz;
            Izlaz = izlaz;
        }

        public string Ime { get;}
        public string Prezime { get;}
        public DateTime Ulaz { get;}
        public DateTime Izlaz { get;}
        public string Id { get;}
    }

    class PresenceReg
    {
        public string Ime { get; set; }
        public string Prezime { get; set; }
        public string Id { get; set; }
    }

    class PresenceTemp
    {
        public PresenceTemp() { }

        public PresenceTemp(string raw)
        {
            var splitLine = raw.Split('|');
            this.Ime = splitLine[0];
            SetVrijeme(splitLine[1]);
            SetUlazIzlaz(splitLine[2]);
        }

        public PresenceTemp(string ime, string vrijeme, string ulazIzlaz)
        {
            Ime = Ime;
            SetVrijeme(vrijeme);
            SetUlazIzlaz(ulazIzlaz);
        }

        public PresenceTemp(string[] raw)
        {
            Ime = raw[0];
            SetVrijeme(raw[1]);
            SetUlazIzlaz(raw[2]);
        }

        public string Ime { get; set; }
        public DateTime Vrijeme { get; set; }//private setter

        public void SetVrijeme(string value)
        {
            DateTime oDate = DateTime.ParseExact(value, "yyyy-MM-ddTHH:mm", null);
            Vrijeme = oDate;
        }

        public bool UlazIzlaz { get; set; }//private setter

        public void SetUlazIzlaz(string value)
        {
            bool oBool = bool.Parse(value);
            UlazIzlaz = oBool;
        }
    }
}
