<?php
/**
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ABN 77 084 670 600                                                 |
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
*
* $Id: saml2_acs.inc,v 1.2 2012/08/30 00:59:03 ewang Exp $
*
*/





require_once SQ_INCLUDE_PATH.'/asset.inc';

/**
* SAML 2.0 Assertion Consumer Service
*
* Purpose
* This asset is exposed as Assertion Consumer Service component for the remote IDP.
* IDP redirects authenticated / semi authenticated user back to this asset for further verification
* 
*
* @author  Edison Wang <ewang@squiz.cm.au>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage saml
*/
class Saml2_ACS extends Asset
{


    /**
    * Constructor
    *
    * @param int    $assetid    the asset id to be loaded
    *
    */
    function __construct($assetid=0)
    {
        parent::__construct($assetid);

    }//end constructor


    /**
    * Returns name of the asset
    *
    * @param boolean    $short_name whether or not we are after the short name or the full name
    *
    * @return string
    * @access private
    * @see Asset::_getName()
    */
    function _getName($short_name=FALSE)
    {
        return $this->attr('name');

    }//end _getName()


    
    /**
    * Perform any additional processing required during the creation of this asset
    *
    * Folders set an initial web path when they are created
    *
    * @param array	&$link	information used to create the initial link
    *
    * @return boolean
    * @access private
    */
    protected function _createAdditional(Array &$link)
    {
	    if (!parent::_createAdditional($link)) return FALSE;

	    // NOTE: the main time we wont want to do this is if we are creating
	    // the root folder as it is not linked anywhere
	    if (empty($link)) return TRUE;

	    return $this->makeAndSaveInitialWebPath(strtolower($this->attr('name')), $link);

    }//end _createAdditional()


    /**
    * Called by the design to print the body of this asset
    *
    * @return void
    * @access public
    */
    function printBody()
    {	
	// get the source id from parent
	$saml_account_manager_data = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'saml_account_manager', TRUE, 'saml2_assertion_consumer_service', 'minor');
	$saml_account_manager = $GLOBALS['SQ_SYSTEM']->am->getAsset($saml_account_manager_data['majorid']);
	
	if(!$saml_account_manager->isSimplesamlphpInstalled()) {
	    echo('simplesamlphp library is not installed');
	    return FALSE;
	}
	
	require_once SQ_TOOL_SIMPLESAMLPHP_PATH.'/lib/_autoload.php';
	require_once SQ_PACKAGES_PATH.'/saml/lib/saml_sp.inc';
	
	// if we are already SAML authenticated, just redirect user to the saml manager url
	// otherwise we are at risk of authenticating again and got lots of error
	$saml_auth_manager = new SAML_Auth_Manager($saml_account_manager->attr('auth_source'));
	if($saml_auth_manager->isAuthenticated()) {
		$redirect_url = $saml_account_manager->getURL();
		if(!empty($redirect_url)) {
			do_redirect($redirect_url);
		}
	}


	$sourceId = $saml_account_manager->attr('auth_source');
	
	$config = SimpleSAML_Configuration::getConfig('authsources.php');
	$authConfig = $config->getArray($sourceId, NULL);
	$source = new SAML_Service_Provider (Array('AuthId' => $sourceId), $authConfig);

	$spMetadata = $source->getMetadata();
	try {
	    $b = SAML2_Binding::getCurrentBinding();
	}
	catch (Exception $e) {
	    echo($e->getMessage());
	    return FALSE;
	}
	
	if ($b instanceof SAML2_HTTPArtifact) {
	   $b->setSPMetadata($spMetadata);
	}

	$response = $b->receive();
	
	if (!($response instanceof SAML2_Response)) {
		throw new SimpleSAML_Error_BadRequest('Invalid message received to AssertionConsumerService endpoint.');
	}

	$idp = $response->getIssuer();
	if ($idp === NULL) {
		/* No Issuer in the response. Look for an unencrypted assertion with an issuer. */
		foreach ($response->getAssertions() as $a) {
			if ($a instanceof SAML2_Assertion) {
				/* We found an unencrypted assertion - there should be an issuer here. */
				$idp = $a->getIssuer();
				break;
			}
		}
		if ($idp === NULL) {
			/* No issuer found in the assertions. */
			throw new Exception('Missing <saml:Issuer> in message delivered to AssertionConsumerService.');
		}
	}

	$session = SimpleSAML_Session::getInstance();
	$prevAuth = $session->getAuthData($sourceId, 'saml:sp:prevAuth');
	if ($prevAuth !== NULL && $prevAuth['id'] === $response->getId() && $prevAuth['issuer'] === $idp) {
		/* OK, it looks like this message has the same issuer
		* and ID as the SP session we already have active. We
		* therefore assume that the user has somehow triggered
		* a resend of the message.
		* In that case we may as well just redo the previous redirect
		* instead of displaying a confusing error message.
		*/
		SimpleSAML_Logger::info('Duplicate SAML 2 response detected - ignoring the response and redirecting the user to the correct page.');
		SimpleSAML_Utilities::redirect($prevAuth['redirect']);
	}

	$stateId = $response->getInResponseTo();
	if (!empty($stateId)) {
		/* This is a response to a request we sent earlier. */
		$state = SimpleSAML_Auth_State::loadState($stateId, 'saml:sp:sso');

		/* Check that the authentication source is correct. */
		assert('array_key_exists("saml:sp:AuthId", $state)');
		if ($state['saml:sp:AuthId'] !== $sourceId) {
			throw new SimpleSAML_Error_Exception('The authentication source id in the URL does not match the authentication source which sent the request.');
		}
	} else {
		/* This is an unsolicited response. */
		$state = array(
			'saml:sp:isUnsolicited' => TRUE,
			'saml:sp:AuthId' => $sourceId,
			'saml:sp:RelayState' => $response->getRelayState(),
		);
	}

	SimpleSAML_Logger::debug('Received SAML2 Response from ' . var_export($idp, TRUE) . '.');

	$idpMetadata = $source->getIdPmetadata($idp);

	try {
		$assertions = sspmod_saml_Message::processResponse($spMetadata, $idpMetadata, $response);
	} catch (sspmod_saml_Error $e) {
		/* The status of the response wasn't "success". 
		 * Redirect back to saml account manager page and display error bodycopy
		 */
		$_SESSION['sq_saml_account_manager_auth_error'] = $e->getStatusMessage();
		$return = $state['SimpleSAML_Auth_Default.Return'];
		if (is_string($return)) {
		    /* Redirect... */
		    SimpleSAML_Utilities::redirect($return);
		}
		assert('FALSE');
		
	}


	$authenticatingAuthority = NULL;
	$nameId = NULL;
	$sessionIndex = NULL;
	$expire = NULL;
	$attributes = array();
	$foundAuthnStatement = FALSE;
	foreach ($assertions as $assertion) {

		/* Check for duplicate assertion (replay attack). */
		$store = SimpleSAML_Store::getInstance();
		if ($store !== FALSE) {
			$aID = $assertion->getId();
			if ($store->get('saml.AssertionReceived', $aID) !== NULL) {
				$e = new SimpleSAML_Error_Exception('Received duplicate assertion.');
				SimpleSAML_Auth_State::throwException($state, $e);
			}

			$notOnOrAfter = $assertion->getNotOnOrAfter();
			if ($notOnOrAfter === NULL) {
				$notOnOrAfter = time() + 24*60*60;
			} else {
				$notOnOrAfter += 60; /* We allow 60 seconds clock skew, so add it here also. */
			}

			$store->set('saml.AssertionReceived', $aID, TRUE, $notOnOrAfter);
		}


		if ($authenticatingAuthority === NULL) {
			$authenticatingAuthority = $assertion->getAuthenticatingAuthority();
		}
		if ($nameId === NULL) {
			$nameId = $assertion->getNameId();
		}
		if ($sessionIndex === NULL) {
			$sessionIndex = $assertion->getSessionIndex();
		}
		if ($expire === NULL) {
			$expire = $assertion->getSessionNotOnOrAfter();
		}

		$attributes = array_merge($attributes, $assertion->getAttributes());

		if ($assertion->getAuthnInstant() !== NULL) {
			/* Assertion contains AuthnStatement, since AuthnInstant is a required attribute. */
			$foundAuthnStatement = TRUE;
		}
	}

	if (!$foundAuthnStatement) {
		$e = new SimpleSAML_Error_Exception('No AuthnStatement found in assertion(s).');
		SimpleSAML_Auth_State::throwException($state, $e);
	}

	if ($expire === NULL) {
		/* Just expire the logout associtaion 24 hours into the future. */
		$expire = time() + 24*60*60;
	}

	/* Register this session in the logout store. */
	sspmod_saml_SP_LogoutStore::addSession($sourceId, $nameId, $sessionIndex, $expire);

	/* We need to save the NameID and SessionIndex for logout. */
	$logoutState = array(
		'saml:logout:Type' => 'saml2',
		'saml:logout:IdP' => $idp,
		'saml:logout:NameID' => $nameId,
		'saml:logout:SessionIndex' => $sessionIndex,
		);
	$state['LogoutState'] = $logoutState;
	$state['saml:AuthenticatingAuthority'] = $authenticatingAuthority;
	$state['saml:AuthenticatingAuthority'][] = $idp;
	$state['PersistentAuthData'][] = 'saml:AuthenticatingAuthority';

	$state['saml:sp:IdP'] = $idp;
	$state['PersistentAuthData'][] = 'saml:sp:IdP';
	$state['saml:sp:NameID'] = $nameId;
	$state['PersistentAuthData'][] = 'saml:sp:NameID';
	$state['saml:sp:SessionIndex'] = $sessionIndex;
	$state['PersistentAuthData'][] = 'saml:sp:SessionIndex';
	$state['saml:sp:AuthnContext'] = $assertion->getAuthnContext();
	$state['PersistentAuthData'][] = 'saml:sp:AuthnContext';


	if (isset($state['SimpleSAML_Auth_Default.ReturnURL'])) {
		/* Just note some information about the authentication, in case we receive the
		* same response again.
		*/
		$state['saml:sp:prevAuth'] = array(
			'id' => $response->getId(),
			'issuer' => $idp,
			'redirect' => $state['SimpleSAML_Auth_Default.ReturnURL'],
		);
		$state['PersistentAuthData'][] = 'saml:sp:prevAuth';
	}

	$source->handleResponse($state, $idp, $attributes);
	assert('FALSE');

    }
	
}//end class
?>
