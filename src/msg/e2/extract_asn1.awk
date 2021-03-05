BEGIN { asn1block = 0; }

/ASN1STOP$/ { asn1block = 0; }

!/Copyright/ && !/O-RAN.WG3.E2AP-v01.01/ && !/_______________________/ && !/^$/ {
  if (asn1block)
    print $0
}

/ASN1START$/ { asn1block = 1; }
