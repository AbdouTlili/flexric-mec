/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "E2AP-IEs"
 * 	found in "e2ap-v01.01.asn1"
 * 	`asn1c -gen-PER -no-gen-OER -fcompound-names -no-gen-example -findirect-choice -fno-include-deps`
 */

#include "E2nodeComponentConfigUpdateGNB.h"

asn_TYPE_member_t asn_MBR_E2nodeComponentConfigUpdateGNB_1[] = {
	{ ATF_POINTER, 4, offsetof(struct E2nodeComponentConfigUpdateGNB, ngAPconfigUpdate),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_OCTET_STRING,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"ngAPconfigUpdate"
		},
	{ ATF_POINTER, 3, offsetof(struct E2nodeComponentConfigUpdateGNB, xnAPconfigUpdate),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_OCTET_STRING,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"xnAPconfigUpdate"
		},
	{ ATF_POINTER, 2, offsetof(struct E2nodeComponentConfigUpdateGNB, e1APconfigUpdate),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_OCTET_STRING,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"e1APconfigUpdate"
		},
	{ ATF_POINTER, 1, offsetof(struct E2nodeComponentConfigUpdateGNB, f1APconfigUpdate),
		(ASN_TAG_CLASS_CONTEXT | (3 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_OCTET_STRING,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"f1APconfigUpdate"
		},
};
static const int asn_MAP_E2nodeComponentConfigUpdateGNB_oms_1[] = { 0, 1, 2, 3 };
static const ber_tlv_tag_t asn_DEF_E2nodeComponentConfigUpdateGNB_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static const asn_TYPE_tag2member_t asn_MAP_E2nodeComponentConfigUpdateGNB_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* ngAPconfigUpdate */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* xnAPconfigUpdate */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 }, /* e1APconfigUpdate */
    { (ASN_TAG_CLASS_CONTEXT | (3 << 2)), 3, 0, 0 } /* f1APconfigUpdate */
};
asn_SEQUENCE_specifics_t asn_SPC_E2nodeComponentConfigUpdateGNB_specs_1 = {
	sizeof(struct E2nodeComponentConfigUpdateGNB),
	offsetof(struct E2nodeComponentConfigUpdateGNB, _asn_ctx),
	asn_MAP_E2nodeComponentConfigUpdateGNB_tag2el_1,
	4,	/* Count of tags in the map */
	asn_MAP_E2nodeComponentConfigUpdateGNB_oms_1,	/* Optional members */
	4, 0,	/* Root/Additions */
	4,	/* First extension addition */
};
asn_TYPE_descriptor_t asn_DEF_E2nodeComponentConfigUpdateGNB = {
	"E2nodeComponentConfigUpdateGNB",
	"E2nodeComponentConfigUpdateGNB",
	&asn_OP_SEQUENCE,
	asn_DEF_E2nodeComponentConfigUpdateGNB_tags_1,
	sizeof(asn_DEF_E2nodeComponentConfigUpdateGNB_tags_1)
		/sizeof(asn_DEF_E2nodeComponentConfigUpdateGNB_tags_1[0]), /* 1 */
	asn_DEF_E2nodeComponentConfigUpdateGNB_tags_1,	/* Same as above */
	sizeof(asn_DEF_E2nodeComponentConfigUpdateGNB_tags_1)
		/sizeof(asn_DEF_E2nodeComponentConfigUpdateGNB_tags_1[0]), /* 1 */
	{ 0, 0, SEQUENCE_constraint },
	asn_MBR_E2nodeComponentConfigUpdateGNB_1,
	4,	/* Elements count */
	&asn_SPC_E2nodeComponentConfigUpdateGNB_specs_1	/* Additional specs */
};

