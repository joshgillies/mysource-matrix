<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd	   |
* | ACN 084 670 600													   |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.														   |
* +--------------------------------------------------------------------+
*
*
*/

require_once SQ_PACKAGES_PATH.'/oauth2/oauth2_token/oauth2_token_edit_fns.inc';

/**
* SugarCRM_Token_Edit_Fns
*
* Purpose
*
* @author  Edison Wang <ewang@squiz.com.au>
* @package MySource_Matrix_Packages
* @subpackage sugar
*/
class SugarCRM_Token_Edit_Fns extends Oauth2_Token_Edit_Fns
{
	/**
	* Constructor
	*
	*/
	function __construct()
	{
		parent::__construct();
		$this->static_screens['details']['force_unlock'] = FALSE;

	}//end constructor


	/**
	* Paints the stored access token
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function paintAccessToken(&$asset, &$o, $prefix)
	{
	    $token = $asset->loadTokenFromStorage();

		?>
		<table border="0" class="sq-backend-table">
			    <th width="150px"><?php echo translate('Access Token'); ?></th>

			    <td>
				<?php
				   echo htmlspecialchars(isset($token['access_token']) ? $token['access_token'] : translate('N/A'));

				?>
			    </td>
			</tr>
			<tr>
			    <th width="150px"><?php echo translate('Expires At'); ?></th>

			    <td>
				<?php
				   $date = isset($token['expires_at']) ? ts_iso8601($token['expires_at']) : translate('N/A');

				   echo htmlspecialchars($date);
				?>
			    </td>
			</tr>
			<tr>
			    <th width="150px"><?php echo translate('Refresh Token'); ?></th>

			    <td>
				<?php
				   echo htmlspecialchars(isset($token['refresh_token']) ? $token['refresh_token'] : translate('N/A'));

				?>
			    </td>
			</tr>
			<tr>
			    <th width="150px"><?php echo translate('Refresh Token Expires At'); ?></th>

			    <td>
				<?php
				   echo htmlspecialchars(isset($token['refresh_token_expires_at']) ? ts_iso8601($token['refresh_token_expires_at']) : translate('N/A'));

				?>
			    </td>
			</tr>
			<tr>
			    <th width="150px"><?php echo translate('Download Token'); ?></th>

			    <td>
				<?php
				   echo htmlspecialchars(isset($token['download_token']) ? $token['download_token'] : translate('N/A'));

				?>
			    </td>
			</tr>
		</table>
	<?php


	}//end paintAccessToken()


	/**
	* Process access token interface
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processAccessToken(&$asset, &$o, $prefix)
	{
		if ($asset->writeAccess('attributes'))  {
		    // wipe out all stored token if configuration changed
		    // because those changed config vars normally means the Oauth provider will invalid all previous access tokens
		    $value = $asset->attr('token_storage');
		    $value .= $asset->attr('scope');
		    $value .= $asset->attr('client_id');
		    $new_config_hash = hash('md5', $value);

		    $old_config_hash = $asset->attr('configuration_hash');
		    if($old_config_hash !== $new_config_hash) {
			$asset->removeAllStoredToken();
			$asset->setAttrValue('configuration_hash', $new_config_hash);
			$asset->saveAttributes();
		    }
		}
	}//end processAccessToken()




	/**
	* Paints the interface for testing API
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function paintTestToken(&$asset, &$o, $prefix)
	{
	   $wa = $asset->writeAccess('attributes');
	   if($wa) {
	   	echo '<label>';
	   	echo check_box($prefix.'_test_token', 1);
	   	echo ' '.translate('Test API Authentication with Access Token').'</label>';

	   	if(isset($_POST[$prefix.'_test_token_status'])) {
	   		if($_POST[$prefix.'_test_token_status']) {
	   			echo '<div><span style="color:green;font-weight:bold">'.translate('Test Successful').'</span></div>';
	   		}
	   		else {
	   			 echo '<div><span style="color:red;font-weight:bold">'.translate('Test Failed').'</span></div>';
	   			 if(isset($_POST[$prefix.'_test_token_response'])) {
	   			 	$response = preg_replace('/[^a-z0-9_:(){}=> ]/i', '', $_POST[$prefix.'_test_token_response']);
	   			 	if (strlen($response) > 200) {
				        $response = substr($response, 0, 200) . "...";
				    }
	   			 	echo $response;
	   			 }
	   		}
	   	}

	   }
	}//end paintTestToken()


	/**
	* Process access token interface
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processTestToken(&$asset, &$o, $prefix)
	{
		if ($asset->writeAccess('attributes'))  {
		    if(isset($_POST[$prefix.'_test_token']) && $_POST[$prefix.'_test_token'] == '1') {
				$token = @$asset->getAccessToken();
				if(isset($token['access_token']) && !empty($token['access_token'])) {
					$access_token = $token['access_token'];
					$api_version = $asset->attr('api_version');
					$base_url = $asset->attr('token_endpoint');
					if(substr($base_url, -1) !== '/') {
						$base_url .= '/';
					}
					$full_url = $base_url.'rest/'.$api_version.'/ping';

					$curl = curl_init($full_url);
					curl_setopt($curl, CURLOPT_HEADER, false);
					curl_setopt($curl, CURLOPT_RETURNTRANSFER, true);
					curl_setopt($curl, CURLOPT_HTTPHEADER, array('Content-Type: application/json',"OAuth-Token: $access_token"));

					// Make the REST call, returning the result
					$response = curl_exec($curl);
					if (!$response || $response != '"pong"') {
						$_POST[$prefix.'_test_token_status'] = false;
						$_POST[$prefix.'_test_token_response'] = translate('Error message').': '.$response;
					   
					}
					else {
						$_POST[$prefix.'_test_token_status'] = true;
					}
				}
				else {
					$_POST[$prefix.'_test_token_status'] = false;
					$_POST[$prefix.'_test_token_response'] = translate('Access Token can not be found');
				}
			}
		}
	}//end processTestToken()



	/**
	* Paints the interface for testing API
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function paintAPIVersion(&$asset, &$o, $prefix)
	{
	   echo $asset->attr('api_version');
	}//end paintAPIVersion()



	/**
	* Paints the interface for Request Token
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function paintRequestNewToken(&$asset, &$o, $prefix)
	{
	   $wa = $asset->writeAccess('attributes');
	   if($wa) {
	   	echo '<label>';
	   	echo check_box($prefix.'_request_new_token', 1);
	   	echo ' '.translate('Request a new Acces Token').'</label>';
	   }
	}//end paintRequestNewToken()


	/**
	* Paints the interface for Request Token
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processRequestNewToken(&$asset, &$o, $prefix)
	{
	   $wa = $asset->writeAccess('attributes');
	   if($wa) {
	   	if(isset($_POST[$prefix.'_request_new_token']) && $_POST[$prefix.'_request_new_token']) {
	   		$asset->getAccessToken(TRUE);
	   	}
	   }
	}//end paintRequestNewToken()

}//end class
?>
