<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ABN 77 084 670 600                                                 |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: saml_auth_manager.inc,v 1.2 2012/08/30 00:59:03 ewang Exp $
*
*/

require_once SQ_PACKAGES_PATH.'/saml/lib/saml_sp.inc';

/**
* SAML_Auth_Manager
*
* Purpose
* This asset is used to  globally manage  Matrix interactions with simplesamlphp library.
* Login, single logout are started from here.
*
*
* @author  Edison Wang <ewang@squiz.com.au>
* @version $Revision: 1.2 $
* @package saml
*/
class SAML_Auth_Manager extends SimpleSAML_Auth_Simple 
{
  
    
    	/**
	 * The id of the authentication source we are accessing.
	 *
	 * @var string
	 */
	protected $authSource;


	/**
	 * Create an instance with the specified authsource.
	 *
	 * @param string $authSource  The id of the authentication source.
	 */
	public function __construct($authSource) {
		$this->authSource = $authSource;
		parent::__construct($authSource);
	}
	
	  /**
	 * Start an authentication process.
	 *
	 * This function never returns.
	 *
	 * This function accepts an array $params, which controls some parts of
	 * the authentication. The accepted parameters depends on the authentication
	 * source being used. Some parameters are generic:
	 *  - 'ErrorURL': An URL that should receive errors from the authentication.
	 *  - 'KeepPost': If the current request is a POST request, keep the POST
	 *    data until after the authentication.
	 *  - 'ReturnTo': The URL the user should be returned to after authentication.
	 *  - 'ReturnCallback': The function we should call after the user has
	 *    finished authentication.
	 *
	* @param array	$params	various login options
	*
	* @return null
	* @access public
	 */
	public function login(array $params = array()) {

		if (array_key_exists('KeepPost', $params)) {
			$keepPost = (bool)$params['KeepPost'];
		} else {
			$keepPost = TRUE;
		}

		if (array_key_exists('ReturnTo', $params)) {
			$returnTo = (string)$params['ReturnTo'];
		} else if (array_key_exists('ReturnCallback', $params)) {
			$returnTo = (array)$params['ReturnCallback'];
		} else {
			$returnTo = current_url();
		}

		if (is_string($returnTo) && $keepPost && $_SERVER['REQUEST_METHOD'] === 'POST') {
			$returnTo = SimpleSAML_Utilities::createPostRedirectLink($returnTo, $_POST);
		}

		if (array_key_exists('ErrorURL', $params)) {
			$errorURL = (string)$params['ErrorURL'];
		} else {
			$errorURL = NULL;
		}


		if (!isset($params[SimpleSAML_Auth_State::RESTART]) && is_string($returnTo)) {
			/*
			 * An URL to restart the authentication, in case the user bookmarks
			 * something, e.g. the discovery service page.
			 */
			$restartURL = $this->getLoginURL($returnTo);
			$params[SimpleSAML_Auth_State::RESTART] = $restartURL;
		}

		$authId = $this->authSource;

		$state = array_merge($params, array(
			'SimpleSAML_Auth_Default.id' => $authId,
			'SimpleSAML_Auth_Default.Return' => $returnTo,
			'SimpleSAML_Auth_Default.ErrorURL' => $errorURL,
			'LoginCompletedHandler' => array('SimpleSAML_Auth_Default', 'loginCompleted'),
			'LogoutCallback' => array(get_class(), 'logoutCallback'),
			'LogoutCallbackState' => array(
				'SimpleSAML_Auth_Default.logoutSource' => $authId,
			),
		));


		if ($errorURL !== NULL) {
			$state[SimpleSAML_Auth_State::EXCEPTION_HANDLER_URL] = $errorURL;
		}
		
		$config = SimpleSAML_Configuration::getConfig('authsources.php');
		$authConfig = $config->getArray($authId, NULL);
	
		$as = new SAML_Service_Provider (Array('AuthId' => $authId), $authConfig);

		if ($as === NULL) {
			throw new Exception('Invalid authentication source: ' . $authId);
		}

		try {
			$as->authenticate($state);
		} catch (SimpleSAML_Error_Exception $e) {
			SimpleSAML_Auth_State::throwException($state, $e);
		} catch (Exception $e) {
			$e = new SimpleSAML_Error_UnserializableException($e);
			SimpleSAML_Auth_State::throwException($state, $e);
		}
		
		SimpleSAML_Auth_Default::loginCompleted($state);
		
		assert('FALSE');
	}
	
	
	
	/**
	 * Log the user out.
	 *
	 * This function logs the user out. It will never return. By default,
	 * it will cause a redirect to the current page after logging the user
	 * out, but a different URL can be given with the $params parameter.
	 *
	 * Generic parameters are:
	 *  - 'ReturnTo': The URL the user should be returned to after logout.
	 *  - 'ReturnCallback': The function that should be called after logout.
	 *  - 'ReturnStateParam': The parameter we should return the state in when redirecting.
	 *  - 'ReturnStateStage': The stage the state array should be saved with.
	 *
	 * @param array	$params	Either the url the user should be redirected to after logging out,
	 *
	 * @return null
	 * @access public
	 */
	public function logout($params = NULL) {
		assert('is_array($params) || is_string($params) || is_null($params)');

		if (empty($params)) {
			$params = current_url();
		}

		if (is_string($params)) {
			$params = array(
				'ReturnTo' => $params,
			);
		}
		
		

		assert('is_array($params)');
		assert('isset($params["ReturnTo"]) || isset($params["ReturnCallback"])');

		if (isset($params['ReturnStateParam']) || isset($params['ReturnStateStage'])) {
			assert('isset($params["ReturnStateParam"]) && isset($params["ReturnStateStage"])');
		}

		$session = SimpleSAML_Session::getInstance();
		if ($session->isValid($this->authSource)) {
			$state = $session->getAuthData($this->authSource, 'LogoutState');
			if ($state !== NULL) {
				$params = array_merge($state, $params);
			}

			$session->doLogout($this->authSource);

			$params['LogoutCompletedHandler'] = array(get_class(), 'logoutCompleted');

			$authId = $this->authSource;
			$config = SimpleSAML_Configuration::getConfig('authsources.php');
			$authConfig = $config->getArray($authId, NULL);

			$as = new SAML_Service_Provider (Array('AuthId' => $authId), $authConfig);

		
			if ($as !== NULL) {
				$as->logout($params);
			}
		}

		self::logoutCompleted($params);
	}
	
	
	/**
	* Generate SP Metadata in XML format
	* @param array	$acs_urls	ACS urls
	* @param array	$sls_urls	SLS urls
	*
	* @return string
	* @access public
	*/
	public  function printSPMetadataXML($acs_urls, $sls_urls)
	{
		$sourceId = $this->authSource;
		
		$config = SimpleSAML_Configuration::getInstance();

		$authConfig = SimpleSAML_Configuration::getConfig('authsources.php');
		$authConfig = $authConfig->getArray($sourceId, NULL);
		$source = new SAML_Service_Provider (Array('AuthId' => $sourceId), $authConfig);

		if ($source === NULL) {
			throw new SimpleSAML_Error_NotFound('Could not find authentication source with id ' . $sourceId);
		}

		if (!($source instanceof sspmod_saml_Auth_Source_SP)) {
			throw new SimpleSAML_Error_NotFound('Source isn\'t a SAML SP: ' . var_export($sourceId, TRUE));
		}

		$entityId = $source->getEntityId();
		$spconfig = $source->getMetadata();

		$ed = new SAML2_XML_md_EntityDescriptor();
		$ed->entityID = $entityId;

		$sp = new SAML2_XML_md_SPSSODescriptor();
		$ed->RoleDescriptor[] = $sp;
		$sp->protocolSupportEnumeration = array(
			'urn:oasis:names:tc:SAML:1.1:protocol',
			'urn:oasis:names:tc:SAML:2.0:protocol'
		);

		foreach ($sls_urls as $sls_url) {
			$slo = new SAML2_XML_md_EndpointType();
			$slo->Binding = SAML2_Const::BINDING_HTTP_REDIRECT;
			$slo->Location = $sls_url;
			$sp->SingleLogoutService[] = $slo;
		}

		$store = SimpleSAML_Store::getInstance();

		if ($store instanceof SimpleSAML_Store_SQL) {
			/* We can properly support SOAP logout. */
			foreach ($sls_urls as $sls_url) {
				$slo = new SAML2_XML_md_EndpointType();
				$slo->Binding = SAML2_Const::BINDING_SOAP;
				$slo->Location = $sls_url;
				$sp->SingleLogoutService[] = $slo;
			}
		}

		$assertionsconsumerservicesdefault = array(
			'urn:oasis:names:tc:SAML:2.0:bindings:HTTP-POST',
			'urn:oasis:names:tc:SAML:1.0:profiles:browser-post',
			'urn:oasis:names:tc:SAML:2.0:bindings:HTTP-Artifact',
			'urn:oasis:names:tc:SAML:1.0:profiles:artifact-01',
			'urn:oasis:names:tc:SAML:2.0:profiles:holder-of-key:SSO:browser',
		);

		$assertionsconsumerservices = $spconfig->getArray('acs.Bindings', $assertionsconsumerservicesdefault);

		$index = 0;
		foreach ($acs_urls as $acs_url) {
			foreach ($assertionsconsumerservices as $services) {
				$acs = new SAML2_XML_md_IndexedEndpointType();
				$acs->index = $index;
				switch ($services) {
				case 'urn:oasis:names:tc:SAML:2.0:bindings:HTTP-POST':
					$acs->Binding = SAML2_Const::BINDING_HTTP_POST;
					$acs->Location = $acs_url;
					break;
				case 'urn:oasis:names:tc:SAML:1.0:profiles:browser-post':
					$acs->Binding = 'urn:oasis:names:tc:SAML:1.0:profiles:browser-post';
					$acs->Location = $acs_url;
					break;
				case 'urn:oasis:names:tc:SAML:2.0:bindings:HTTP-Artifact':
					$acs->Binding = 'urn:oasis:names:tc:SAML:2.0:bindings:HTTP-Artifact';
					$acs->Location = $acs_url;
					break;
				case 'urn:oasis:names:tc:SAML:1.0:profiles:artifact-01':
					$acs->Binding = 'urn:oasis:names:tc:SAML:1.0:profiles:artifact-01';
					$acs->Location = $acs_url;
					break;
				case 'urn:oasis:names:tc:SAML:2.0:profiles:holder-of-key:SSO:browser':
					$acs->Binding = 'urn:oasis:names:tc:SAML:2.0:profiles:holder-of-key:SSO:browser';
					$acs->ProtocolBinding = SAML2_Const::BINDING_HTTP_POST;
					$acs->Location = $acs_url;
					break;
				}
				$sp->AssertionConsumerService[] = $acs;
				$index++;
			}
		}


		$keys = array();
		$certInfo = SimpleSAML_Utilities::loadPublicKey($spconfig, FALSE, 'new_');
		if ($certInfo !== NULL && array_key_exists('certData', $certInfo)) {
			$hasNewCert = TRUE;

			$certData = $certInfo['certData'];
			$kd = SAML2_Utils::createKeyDescriptor($certData);
			$kd->use = 'signing';
			$sp->KeyDescriptor[] = $kd;

			$kd = SAML2_Utils::createKeyDescriptor($certData);
			$kd->use = 'encryption';
			$sp->KeyDescriptor[] = $kd;

			$keys[] = array(
				'type' => 'X509Certificate',
				'signing' => TRUE,
				'encryption' => TRUE,
				'X509Certificate' => $certInfo['certData'],
			);
		} else {
			$hasNewCert = FALSE;
		}

		$certInfo = SimpleSAML_Utilities::loadPublicKey($spconfig);
		if ($certInfo !== NULL && array_key_exists('certData', $certInfo)) {
			$certData = $certInfo['certData'];
			$kd = SAML2_Utils::createKeyDescriptor($certData);
			$kd->use = 'signing';
			$sp->KeyDescriptor[] = $kd;

			if (!$hasNewCert) {
				/* Don't include the old certificate for encryption when we have a newer certificate. */
				$kd = SAML2_Utils::createKeyDescriptor($certData);
				$kd->use = 'encryption';
				$sp->KeyDescriptor[] = $kd;
			}

			$keys[] = array(
				'type' => 'X509Certificate',
				'signing' => TRUE,
				'encryption' => ($hasNewCert ? FALSE : TRUE),
				'X509Certificate' => $certInfo['certData'],
			);
		} else {
			$certData = NULL;
		}

		$name = $spconfig->getLocalizedString('name', NULL);
		$attributes = $spconfig->getArray('attributes', array());

		if ($name !== NULL && !empty($attributes)) {

			$attributesrequired = $spconfig->getArray('attributes.required', array());

			/* We have everything necessary to add an AttributeConsumingService. */
			$acs = new SAML2_XML_md_AttributeConsumingService();
			$sp->AttributeConsumingService[] = $acs;

			$acs->index = 0;
			$acs->ServiceName = $name;

			$description = $spconfig->getLocalizedString('description', NULL);
			if ($description !== NULL) {
				$acs->ServiceDescription = $description;
			}

			$nameFormat = $spconfig->getString('attributes.NameFormat', NULL);
			foreach ($attributes as $attribute) {
				$a = new SAML2_XML_md_RequestedAttribute();
				$a->Name = $attribute;
				$a->NameFormat = $nameFormat;
				// Is the attribute required
				if (in_array($attribute, $attributesrequired))
					$a->isRequired = true;

				$acs->RequestedAttribute[] = $a;
			}
		}

		$orgName = $spconfig->getLocalizedString('OrganizationName', NULL);
		if ($orgName !== NULL) {
			$o = new SAML2_XML_md_Organization();
			$o->OrganizationName = $orgName;

			$o->OrganizationDisplayName = $spconfig->getLocalizedString('OrganizationDisplayName', NULL);
			if ($o->OrganizationDisplayName === NULL) {
				$o->OrganizationDisplayName = $orgName;
			}

			$o->OrganizationURL = $spconfig->getLocalizedString('OrganizationURL', NULL);
			if ($o->OrganizationURL === NULL) {
				throw new SimpleSAML_Error_Exception('If OrganizationName is set, OrganizationURL must also be set.');
			}

			$ed->Organization = $o;
		}

		$c = new SAML2_XML_md_ContactPerson();
		$c->contactType = 'technical';

		$email = $config->getString('technicalcontact_email', NULL);
		if ($email !== NULL) {
			$c->EmailAddress = array($email);
		}

		$name = $config->getString('technicalcontact_name', NULL);
		if ($name === NULL) {
			/* Nothing to do here... */
		} elseif (preg_match('@^(.*?)\s*,\s*(.*)$@D', $name, $matches)) {
			$c->SurName = $matches[1];
			$c->GivenName = $matches[2];
		} elseif (preg_match('@^(.*?)\s+(.*)$@D', $name, $matches)) {
			$c->GivenName = $matches[1];
			$c->SurName = $matches[2];
		} else {
			$c->GivenName = $name;
		}
		$ed->ContactPerson[] = $c;

		$xml = $ed->toXML();
		SimpleSAML_Utilities::formatDOMElement($xml);
		$xml = $xml->ownerDocument->saveXML($xml);

		/* Sign the metadata if enabled. */
		$xml = SimpleSAML_Metadata_Signer::sign($xml, $sp, 'SAML 2 SP');

		return $xml;
	}//end printSPMetadataXML()
	
	
	
}


?>
