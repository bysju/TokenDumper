
#include <string>
#include <vector>
#include <AclAPI.h>
#include <rapidjson/prettywriter.h>


#include "Auxiliary.h"
namespace tokenDumper {

	template<typename PresentTrait>
	typename PresentTrait::InfoType TokenDumper<PresentTrait>::Dump(const BYTE* data, TOKEN_INFORMATION_CLASS infoClass) {


		switch (infoClass) {
		case TokenUser: {
			return DumpTokenUser(data);
		}
		case TokenGroups: {
			return DumpTokenGroups(data);
		}
		case TokenPrivileges: {
			return DumpTokenPrivileges(data);
		}
		case TokenOwner: {
			return DumpTokenOwner(data);
		}
		case TokenPrimaryGroup: {
			return DumpTokenPrimaryGroup(data);
		}
		case TokenDefaultDacl: {
			return DumpTokenDefaultDacl(data);
		}
		case TokenIntegrityLevel: {
			return DumpTokenIntegrityLevel(data);
		}
			default:{

				std::stringstream ss;
				ss << TokenInformationClassToString(infoClass) << " is not supported.";
				throw std::runtime_error(ss.str());
			}
		
		}


	}

	template<typename PresentTrait>
	typename PresentTrait::InfoType TokenDumper<PresentTrait>::DumpTokenUser(const BYTE* data) {

		const TOKEN_USER * user = reinterpret_cast<const TOKEN_USER*>(data);

		PresentTrait trait;
		trait.Start("User");

		// Get a SID
		std::string strSid = ConvertSidToString(user->User.Sid);
		trait.AddItem("Sid", strSid.c_str(), FALSE, FALSE);

		// Get an attribute
		trait.AddItem("Attributes", std::to_string(user->User.Attributes).c_str(), TRUE, FALSE);

		return trait.End();

	}

	template<typename PresentTrait>
	typename PresentTrait::InfoType TokenDumper<PresentTrait>::DumpTokenGroups(const BYTE* data) {

		const TOKEN_GROUPS* groups = reinterpret_cast<const TOKEN_GROUPS*>(data);

		PresentTrait trait;
		trait.Start("Groups");

		DWORD groupCount = groups->GroupCount;

		for (DWORD index = 0; index < groupCount; ++index) {

			SID_AND_ATTRIBUTES groupSidAndAttributes = groups->Groups[index];

			std::string groupName = LookupAccount(groupSidAndAttributes.Sid);

			trait.OpenGroup(groupName.c_str());

			// Get a SID
			std::string strSid = ConvertSidToString(groupSidAndAttributes.Sid);
			trait.AddItem("Sid", strSid.c_str(), FALSE, TRUE);

			// Get an attribute
			std::vector<std::string> strAttributes = GroupAttributesToStringVec(groupSidAndAttributes.Attributes);
			std::string strDetailedAttributes = AttributesToString( groupSidAndAttributes.Attributes, FALSE, strAttributes);

			trait.AddItem("Attributes", strDetailedAttributes.c_str(), FALSE, FALSE);
			trait.CloseGroup();

		}

		return trait.End();
	}

	template<typename PresentTrait>
	typename PresentTrait::InfoType TokenDumper<PresentTrait>::DumpTokenPrivileges(const BYTE* data) {

		const TOKEN_PRIVILEGES* privileges = reinterpret_cast<const TOKEN_PRIVILEGES*>(data);

		PresentTrait trait;
		trait.Start("Privilegs");

		for (DWORD index = 0; index < privileges->PrivilegeCount; ++index) {

			LUID_AND_ATTRIBUTES privilegeAndAttributes = privileges->Privileges[index];

			//Convert privilege to string
			std::string strPrivilege = ConvertLuidToString(&(privilegeAndAttributes.Luid));
			trait.OpenGroup(strPrivilege.c_str());

			// Get an attribute
			std::vector<std::string> strAttributes = PrivilegeAttributesToStringVec(privilegeAndAttributes.Attributes);
			std::string strDetailedAttributes= AttributesToString(privilegeAndAttributes.Attributes, FALSE, strAttributes);

			trait.AddItem("Attributes", strDetailedAttributes.c_str() , FALSE, FALSE);
			trait.CloseGroup();
		}

		return trait.End();

	}

	template<typename PresentTrait>
	typename PresentTrait::InfoType TokenDumper<PresentTrait>::DumpTokenOwner(const BYTE* data) {

		const TOKEN_OWNER* owner = reinterpret_cast<const TOKEN_OWNER*>(data);

		PresentTrait trait;
		trait.Start("Onwer");

		std::string strAccount = LookupAccount(owner->Owner);
		trait.AddItem("Account", strAccount.c_str(), FALSE, FALSE);
		std::string strSid = ConvertSidToString(owner->Owner);
		trait.AddItem("Sid", strSid.c_str(), FALSE, FALSE);


		return trait.End();
	}

	template<typename PresentTrait>
	typename PresentTrait::InfoType TokenDumper<PresentTrait>::DumpTokenPrimaryGroup(const BYTE* data) {

		const TOKEN_PRIMARY_GROUP* primaryGroup = reinterpret_cast<const TOKEN_PRIMARY_GROUP*>(data);

		PresentTrait trait;
		trait.Start("PrimaryGroup");

		std::string strAccount = LookupAccount(primaryGroup->PrimaryGroup);
		trait.AddItem("Group", strAccount.c_str(), FALSE, FALSE);
		std::string strSid = ConvertSidToString(primaryGroup->PrimaryGroup);
		trait.AddItem("Sid", strSid.c_str(), FALSE, FALSE);


		return trait.End();

	}

	template<typename PresentTrait>
	typename PresentTrait::InfoType TokenDumper<PresentTrait>::DumpTokenDefaultDacl(const BYTE* data) {

		const TOKEN_DEFAULT_DACL* defaultAcl = reinterpret_cast<const TOKEN_DEFAULT_DACL*>(data);

		PresentTrait trait;
		trait.Start("DefaultDacl");

		// Get ACEs
		ULONG count{ 0 };
		PEXPLICIT_ACCESS_A  pAccess{ nullptr };
		DWORD ret{ ERROR_SUCCESS };
		if (ERROR_SUCCESS != (ret =  GetExplicitEntriesFromAclA(defaultAcl->DefaultDacl, &count, &pAccess))) {

			throw win32_exception(ret, "Failed to call GetExplicitEntriesFromAclA in DumpTokenDefaultDacl");
		}

		std::vector<ULONG> permissionIndexes, auditIndexes;
		for (ULONG index = 0; index < count; ++index) {

			if (SET_AUDIT_SUCCESS == pAccess[index].grfAccessMode || SET_AUDIT_FAILURE == pAccess[index].grfAccessMode) {
				auditIndexes.push_back(index);
			}
			else {
				permissionIndexes.push_back(index);
			}
		}

		trait.OpenGroup("Permissions");

		auto ListAces = [&trait, &pAccess](ULONG index) {

			std::string strPrincipal = TrusteeToString(&(pAccess[index].Trustee));
			trait.OpenGroup(strPrincipal.c_str());

			// Type
			// TODO: type is alwasy unknwon. I have not yet to find the reason.
			// I think I have to use another API.
			trait.AddItem("Type", TrusteeTypeToString(pAccess[index].Trustee.TrusteeType).c_str(), FALSE, TRUE);

			// Access Mask
			std::vector<std::string> strAccessMasks= AccessMaskToStringVec(pAccess[index].grfAccessPermissions);
			std::string strDetailedAccessMasks = AttributesToString(pAccess[index].grfAccessPermissions, TRUE, strAccessMasks );
			trait.AddItem("AccessMasks", strDetailedAccessMasks.c_str(), FALSE, FALSE);

			// Access Mode
			trait.AddItem("AccessMode", AccessModeToString(pAccess[index].grfAccessMode).c_str(), FALSE, FALSE);

			// Inheritance
			std::vector<std::string> strInheritances = AcesssInheritanceToStringVec(pAccess[index].grfInheritance);
			std::string strDetailedInheritances = AttributesToString(pAccess[index].grfInheritance, TRUE, strInheritances);
			trait.AddItem("Inheritance", strDetailedInheritances.c_str(), FALSE, FALSE);

			trait.CloseGroup();
		};

		for (ULONG index : permissionIndexes) {

			ListAces(index);
		}

		trait.CloseGroup();

		trait.OpenGroup("Auditing");

		for (ULONG index: auditIndexes) {

			ListAces(index);
		}

		trait.CloseGroup();

		return trait.End();
	}

	template<typename PresentTrait>
	typename PresentTrait::InfoType TokenDumper<PresentTrait>::DumpTokenIntegrityLevel(const BYTE* data) {

		const TOKEN_MANDATORY_LABEL* mandatoryLevel = reinterpret_cast<const TOKEN_MANDATORY_LABEL*>(data);

		PresentTrait trait;
		trait.Start("IntegrityLevel");

		// S-1-16-x stands for the mandatory level
		// The last number(x) is an actual level.
		// 
		// Get a SID
		std::string strSid = ConvertSidToString(mandatoryLevel->Label.Sid);
		DWORD level = *GetSidSubAuthority(mandatoryLevel->Label.Sid,(DWORD)(UCHAR)(*GetSidSubAuthorityCount(mandatoryLevel->Label.Sid) - 1));
		
		if (SECURITY_MANDATORY_UNTRUSTED_RID == level)
			strSid += " (SECURITY_MANDATORY_UNTRUSTED_RID)";
		else if(SECURITY_MANDATORY_LOW_RID == level)
			strSid += " (SECURITY_MANDATORY_LOW_RID)";
		else if(SECURITY_MANDATORY_MEDIUM_RID)
			strSid += " (SECURITY_MANDATORY_MEDIUM_RID)";
		else if(SECURITY_MANDATORY_MEDIUM_PLUS_RID)
			strSid += " (SECURITY_MANDATORY_MEDIUM_PLUS_RID)";
		else if(SECURITY_MANDATORY_HIGH_RID)
			strSid += " (SECURITY_MANDATORY_HIGH_RID)";
		else if(SECURITY_MANDATORY_SYSTEM_RID)
			strSid += " (SECURITY_MANDATORY_SYSTEM_RID)";
		else if(SECURITY_MANDATORY_PROTECTED_PROCESS_RID)
			strSid += " (SECURITY_MANDATORY_PROTECTED_PROCESS_RID)";

		trait.AddItem("Sid", strSid.c_str(), FALSE, FALSE);

		// Get an attribute
		trait.AddItem("Attributes", std::to_string(mandatoryLevel->Label.Attributes).c_str(), TRUE, FALSE);

		return trait.End();
	}


};
