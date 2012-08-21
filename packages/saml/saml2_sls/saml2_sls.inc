<?php
/**
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
*
* $Id: saml2_sls.inc,v 1.1.1.1 2012/08/21 05:17:13 ewang Exp $
*
*/





require_once SQ_INCLUDE_PATH.'/asset.inc';

/**
* SAML 2.0 Single Logout Service
*
* Purpose
* This asset is exposed as Single Logout Service component for the remote IDP.
* IDP redirects authenticated / semi authenticated user back to this asset for further action
*
*
* @author  Edison Wang <ewang@squiz.cm.au>
* @version $Revision: 1.1.1.1 $
* @package MySource_Matrix_Packages
* @subpackage saml
*/
class Saml2_SLS extends Asset
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
	$saml_account_manager_data = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'saml_account_manager', TRUE, 'saml2_single_logout_service', 'minor');
	$saml_account_manager = $GLOBALS['SQ_SYSTEM']->am->getAsset($saml_account_manager_data['majorid']);
	
	if(!$saml_account_manager->isSimplesamlphpInstalled()) {
	    echo('simplesamlphp library is not installed');
	    return FALSE;
	}
	
	require_once SQ_TOOL_SIMPLESAMLPHP_PATH.'/lib/_autoload.php';
	require_once SQ_PACKAGES_PATH.'/saml/lib/saml_auth_manager.inc';
	require_once SQ_PACKAGES_PATH.'/saml/lib/saml_sp.inc';
	
	$sourceId = $saml_account_manager->attr('auth_source');
	
	$config = SimpleSAML_Configuration::getConfig('authsources.php');
	$authConfig = $config->getArray($sourceId, NULL);
	$source = new SAML_Service_Provider (Array('AuthId' => $sourceId), $authConfig);

	try {
	    $binding = SAML2_Binding::getCurrentBinding();
	}
	catch (Exception $e) {
	    echo($e->getMessage());
	    return FALSE;
	}
	
	$binding = SAML2_Binding::getCurrentBinding();
	$message = $binding->receive();

	$idpEntityId = $message->getIssuer();
	if ($idpEntityId === NULL) {
		/* Without an issuer we have no way to respond to the message. */
		throw new SimpleSAML_Error_BadRequest('Received message on logout endpoint without issuer.');
	}

	$spEntityId = $source->getEntityId();

	$metadata = SimpleSAML_Metadata_MetaDataStorageHandler::getMetadataHandler();
	$idpMetadata = $source->getIdPMetadata($idpEntityId);
	$spMetadata = $source->getMetadata();

	sspmod_saml_Message::validateMessage($idpMetadata, $spMetadata, $message);

	$destination = $message->getDestination();
	if ($destination !== NULL && $destination !== SimpleSAML_Utilities::selfURLNoQuery()) {
		throw new SimpleSAML_Error_Exception('Destination in logout message is wrong.');
	}

	if ($message instanceof SAML2_LogoutResponse) {

		$relayState = $message->getRelayState();
		if ($relayState === NULL) {
			/* Somehow, our RelayState has been lost. */
			throw new SimpleSAML_Error_BadRequest('Missing RelayState in logout response.');
		}

		if (!$message->isSuccess()) {
			SimpleSAML_Logger::warning('Unsuccessful logout. Status was: ' . sspmod_saml_Message::getResponseError($message));
		}

		$state = SimpleSAML_Auth_State::loadState($relayState, 'saml:slosent');
		$state['saml:sp:LogoutStatus'] = $message->getStatus();
		SimpleSAML_Auth_Source::completeLogout($state);

	} elseif ($message instanceof SAML2_LogoutRequest) {

		SimpleSAML_Logger::debug('SAML2 Single Logout Service: Request from ' . $idpEntityId);
		SimpleSAML_Logger::stats('saml20-idp-SLO idpinit ' . $spEntityId . ' ' . $idpEntityId);

		if ($message->isNameIdEncrypted()) {
			try {
				$keys = sspmod_saml_Message::getDecryptionKeys($idpMetadata, $spMetadata);
			} catch (Exception $e) {
				throw new SimpleSAML_Error_Exception('Error decrypting NameID: ' . $e->getMessage());
			}

			$lastException = NULL;
			foreach ($keys as $i => $key) {
				try {
					$message->decryptNameId($key);
					SimpleSAML_Logger::debug('Decryption with key #' . $i . ' succeeded.');
					$lastException = NULL;
					break;
				} catch (Exception $e) {
					SimpleSAML_Logger::debug('Decryption with key #' . $i . ' failed with exception: ' . $e->getMessage());
					$lastException = $e;
				}
			}
			if ($lastException !== NULL) {
				throw $lastException;
			}
		}

		$nameId = $message->getNameId();
		$sessionIndexes = $message->getSessionIndexes();

		$numLoggedOut = sspmod_saml_SP_LogoutStore::logoutSessions($sourceId, $nameId, $sessionIndexes);
		if ($numLoggedOut === FALSE) {
			/* This type of logout was unsupported. Use the old method. */
			$source->handleLogout($idpEntityId);
			$numLoggedOut = count($sessionIndexes);
		}

		/* Create an send response. */
		$lr = sspmod_saml_Message::buildLogoutResponse($spMetadata, $idpMetadata);
		$lr->setRelayState($message->getRelayState());
		$lr->setInResponseTo($message->getId());

		if ($numLoggedOut < count($sessionIndexes)) {
			SimpleSAML_Logger::warning('Logged out of ' . $numLoggedOut  . ' of ' . count($sessionIndexes) . ' sessions.');
		}

		$binding->send($lr);
	} else {
		throw new SimpleSAML_Error_BadRequest('Unknown message received on logout endpoint: ' . get_class($message));
	}


    }
	
}//end class
?>
