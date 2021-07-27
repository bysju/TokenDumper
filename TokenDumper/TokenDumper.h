#pragma once
#include <ostream>
#include <Windows.h>

namespace tokenDumper {


	template<typename PresentTrait>
	class TokenDumper{
	public:
		explicit TokenDumper() {
		}
		void Dump(const BYTE* data, TOKEN_INFORMATION_CLASS infoClass, std::ostream & os);

	private:
		// Dump each class
		typename PresentTrait::InfoType DumpTokenUser(const BYTE* data);
		//TokenGroups,
		//TokenPrivileges,
		//TokenOwner,
		//TokenPrimaryGroup,
		//TokenDefaultDacl,
		//TokenSource,
		//TokenType,
		//TokenImpersonationLevel,
		//TokenStatistics,
		//TokenRestrictedSids,
		//TokenSessionId,
		//TokenGroupsAndPrivileges,
		//TokenSessionReference,
		//TokenSandBoxInert,
		//TokenAuditPolicy,
		//TokenOrigin,
		//TokenElevationType,
		//TokenLinkedToken,
		//TokenElevation,
		//TokenHasRestrictions,
		//TokenAccessInformation,
		//TokenVirtualizationAllowed,
		//TokenVirtualizationEnabled,
		typename PresentTrait::InfoType DumpTokenIntegrityLevel(const BYTE* data);
		//TokenUIAccess,
		//TokenMandatoryPolicy,
		//TokenLogonSid,
		//TokenIsAppContainer,
		//TokenCapabilities,
		//TokenAppContainerSid,
		//TokenAppContainerNumber,
		//TokenUserClaimAttributes,
		//TokenDeviceClaimAttributes,
		//TokenRestrictedUserClaimAttributes,
		//TokenRestrictedDeviceClaimAttributes,
		//TokenDeviceGroups,
		//TokenRestrictedDeviceGroups,
		//TokenSecurityAttributes,
		//TokenIsRestricted,
		//TokenProcessTrustLevel,
		//TokenPrivateNameSpace,
		//TokenSingletonAttributes,
		//TokenBnoIsolation,
		//TokenChildProcessFlags,
		//TokenIsLessPrivilegedAppContainer,
		//TokenIsSandboxed,
		//TokenOriginatingProcessTrustLevel,
		//MaxTokenInfoClass  
	private:
	};
};

#include "TokenDumper.hpp"