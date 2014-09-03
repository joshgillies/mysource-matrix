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
* $Id: saml_sp.inc,v 1.4 2012/08/30 00:59:03 ewang Exp $
*
*/

/**
* SAML_Service_Provider
*
* Purpose
* This class extends simplesamlphp SP class and manages specific actions during SAML auth process.
* It does heavy liftings to construct auth request messages.
*
*
* @author  Edison Wang <ewang@squiz.com.au>
* @version $Revision: 1.4 $
* @package saml
*/
class SAML_Service_Provider extends sspmod_saml_Auth_Source_SP 
{
    

    
    	/**
	 * The entity ID of this SP.
	 *
	 * @var string
	 */
	public $entityId;


	/**
	 * The metadata of this SP.
	 *
	 * @var SimpleSAML_Configuration.
	 */
	public $metadata;


	/**
	 * The IdP the user is allowed to log into.
	 *
	 * @var string|NULL  The IdP the user can log into, or NULL if the user can log into all IdPs.
	 */
	public $idp;


	/**
	 * URL to discovery service.
	 *
	 * @var string|NULL
	 */
	public $discoURL;


	/**
	 * Constructor for SAML SP authentication source.
	 *
	 * @param array $info  Information about this authentication source.
	 * @param array $config  Configuration.
	 */
	public function __construct($info, $config) {
		assert('is_array($info)');
		assert('is_array($config)');

		/* Call the parent constructor first, as required by the interface. */
		parent::__construct($info, $config);

		if(!isset($config[0]) || $config[0] !== 'saml:SP') {
		    trigger_error('The selected authentication source is not SAML Service Provider');
		    exit();
		}
		
		if(!isset($config['entityID'])) {
		    trigger_error('entityID is not found in the selected authentication source');
		    exit();
		}

		/* For compatibility with code that assumes that $metadata->getString('entityid') gives the entity id. */
		$config['entityid'] = $config['entityID'];

		$this->metadata = SimpleSAML_Configuration::loadFromArray($config, 'authsources[' . var_export($this->authId, TRUE) . ']');
		$this->entityId = $this->metadata->getString('entityID');
		$this->idp = $this->metadata->getString('idp', NULL);
		$this->discoURL = $this->metadata->getString('discoURL', NULL);
		
		if (empty($this->discoURL) && SimpleSAML_Module::isModuleEnabled('discojuice')) {
			$this->discoURL = SimpleSAML_Module::getModuleURL('discojuice/central.php');
		}
	}

	
	
	/**
	 * Send a SSO request to an IdP.
	 *
	 * @param string $idp  The entity ID of the IdP.
	 * @param array $state  The state array for the current authentication.
	 */
	public function startSSO($idp, array $state) {
		assert('is_string($idp)');

		$idpMetadata = $this->getIdPMetadata($idp);

		$type = $idpMetadata->getString('metadata-set');
		switch ($type) {
		case 'saml20-idp-remote':
			$this->startSSO2($idpMetadata, $state);
			assert('FALSE'); /* Should not return. */
		default:
			/* Should only be one of the known types. */
			assert('FALSE');
		}
	}
    

	
	
	 /**
	 * Send a SAML2 SSO request to an IdP.
	 *
	 * @param SimpleSAML_Configuration $idpMetadata  The metadata of the IdP.
	 * @param array $state  The state array for the current authentication.
	 */
	public function startSSO2(SimpleSAML_Configuration $idpMetadata, array $state) {
		if (isset($state['saml:ProxyCount']) && $state['saml:ProxyCount'] < 0) {
			SimpleSAML_Auth_State::throwException($state, new SimpleSAML_Error_ProxyCountExceeded("ProxyCountExceeded"));
		}

		$ar = sspmod_saml_Message::buildAuthnRequest($this->metadata, $idpMetadata);

		// Set ACS URL as Matrix's ACS asset
		$ar->setAssertionConsumerServiceURL($state['acs_url']);

		if (isset($state['SimpleSAML_Auth_Default.ReturnURL'])) {
			$ar->setRelayState($state['SimpleSAML_Auth_Default.ReturnURL']);
		}

		if (isset($state['saml:AuthnContextClassRef'])) {
			$accr = SimpleSAML_Utilities::arrayize($state['saml:AuthnContextClassRef']);
			$ar->setRequestedAuthnContext(array('AuthnContextClassRef' => $accr));
		}

		if (isset($state['ForceAuthn'])) {
			$ar->setForceAuthn((bool)$state['ForceAuthn']);
		}

		if (isset($state['isPassive'])) {
			$ar->setIsPassive((bool)$state['isPassive']);
		}
		
		if (isset($state['ProtocolBinding'])) {
			$ar->setProtocolBinding($state['ProtocolBinding']);
		}
		
		if (isset($state['saml:NameIDPolicy'])) {
			if (is_string($state['saml:NameIDPolicy'])) {
				$policy = array(
					'Format' => (string)$state['saml:NameIDPolicy'],
					'AllowCreate' => TRUE,
				);
			} elseif (is_array($state['saml:NameIDPolicy'])) {
				$policy = $state['saml:NameIDPolicy'];
			} else {
				throw new SimpleSAML_Error_Exception('Invalid value of $state[\'saml:NameIDPolicy\'].');
			}
			$ar->setNameIdPolicy($policy);
		}

		if (isset($state['saml:IDPList'])) {
			$IDPList = $state['saml:IDPList'];
		} else {
		    $IDPList = array();
		}

		$ar->setIDPList(array_unique(array_merge($this->metadata->getArray('IDPList', array()), 
												$idpMetadata->getArray('IDPList', array()),
												(array) $IDPList)));
		
		if (isset($state['saml:ProxyCount']) && $state['saml:ProxyCount'] !== null) {
			$ar->setProxyCount($state['saml:ProxyCount']);
		} elseif ($idpMetadata->getInteger('ProxyCount', null) !== null) {
			$ar->setProxyCount($idpMetadata->getInteger('ProxyCount', null));
		} elseif ($this->metadata->getInteger('ProxyCount', null) !== null) {
			$ar->setProxyCount($this->metadata->getInteger('ProxyCount', null));
		}
		
		$requesterID = array();
		if (isset($state['saml:RequesterID'])) {
			$requesterID = $state['saml:RequesterID'];
		}
		
		if (isset($state['core:SP'])) {
			$requesterID[] = $state['core:SP'];
		}
		
		$ar->setRequesterID($requesterID);
		
		if (isset($state['saml:Extensions'])) {
			$ar->setExtensions($state['saml:Extensions']);
		}

		$id = SimpleSAML_Auth_State::saveState($state, 'saml:sp:sso', TRUE);
		$ar->setId($id);

		SimpleSAML_Logger::debug('Sending SAML 2 AuthnRequest to ' . var_export($idpMetadata->getString('entityid'), TRUE));


		/* Select appropriate SSO endpoint */
		if ($ar->getProtocolBinding() === SAML2_Const::BINDING_HOK_SSO) {
			$dst = $idpMetadata->getDefaultEndpoint('SingleSignOnService', array(
				SAML2_Const::BINDING_HOK_SSO)
			);
		} else {
			$dst = $idpMetadata->getDefaultEndpoint('SingleSignOnService', array(
				SAML2_Const::BINDING_HTTP_REDIRECT,
				SAML2_Const::BINDING_HTTP_POST)
			);
		}
		$ar->setDestination($dst['Location']);



		if ($this->metadata->getValue('ProtocolBinding') === 'urn:oasis:names:tc:SAML:2.0:bindings:HTTP-POST') {
			    $b = new SAML2_HTTPPost();
		    } else {
			    $b = new SAML2_HTTPRedirect();
		    }
		$b->send($ar);

		assert('FALSE');
	}// end startSSO2()
	
	
	
	/**
	 * Start a SAML 2 logout operation.
	 *
	 * @param array $state  The logout state.
	 */
	public function startSLO2(&$state) {
		assert('is_array($state)');
		assert('array_key_exists("saml:logout:IdP", $state)');
		assert('array_key_exists("saml:logout:NameID", $state)');
		assert('array_key_exists("saml:logout:SessionIndex", $state)');

		$id = SimpleSAML_Auth_State::saveState($state, 'saml:slosent');

		$idp = $state['saml:logout:IdP'];
		$nameId = $state['saml:logout:NameID'];
		$sessionIndex = $state['saml:logout:SessionIndex'];

		$idpMetadata = $this->getIdPMetadata($idp);

		$endpoint = $idpMetadata->getDefaultEndpoint('SingleLogoutService', array(SAML2_Const::BINDING_HTTP_REDIRECT), FALSE);
		if ($endpoint === FALSE) {
			SimpleSAML_Logger::info('No logout endpoint for IdP ' . var_export($idp, TRUE) . '.');
			return;
		}
		
		$lr = sspmod_saml_Message::buildLogoutRequest($this->metadata, $idpMetadata);
		$lr->setNameId($nameId);
		$lr->setSessionIndex($sessionIndex);
		$lr->setRelayState($id);
		$lr->setDestination($endpoint['Location']);
		

		$encryptNameId = $idpMetadata->getBoolean('nameid.encryption', NULL);
		if ($encryptNameId === NULL) {
			$encryptNameId = $this->metadata->getBoolean('nameid.encryption', FALSE);
		}
		if ($encryptNameId) {
			$lr->encryptNameId(sspmod_saml_Message::getEncryptionKey($idpMetadata));
		}

		if ($this->metadata->getValue('ProtocolBinding') === 'urn:oasis:names:tc:SAML:2.0:bindings:HTTP-POST') {
			    $b = new SAML2_HTTPPost();
		} else {
			    $b = new SAML2_HTTPRedirect();
		}
		$b->send($lr);

		assert('FALSE');
	}// end startSLO2()
	
	
	

}// end class


?>
