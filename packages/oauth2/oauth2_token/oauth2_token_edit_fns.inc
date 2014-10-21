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
* $Id: oauth2_token_edit_fns.inc,v 1.1.1.1 2013/07/19 00:29:26 ewang Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/page/page_edit_fns.inc';

/**
* Oauth2_Token_Edit_Fns
*
* Purpose
*
* @author  Edison Wang <ewang@squiz.com.au>
* @version $ $
* @package MySource_Matrix_Packages
* @subpackage oauth2
*/
class Oauth2_Token_Edit_Fns extends Page_Edit_Fns
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
			    <th width="20%"><?php echo translate('Access Token'); ?></th>

			    <td>
				<?php
				   echo htmlspecialchars(isset($token['access_token']) ? $token['access_token'] : translate('N/A'));

				?>
			    </td>
			</tr>
			<tr>
			    <th width="20%"><?php echo translate('Expires At'); ?></th>

			    <td>
				<?php
				   $date = isset($token['expires_at']) ? ts_iso8601($token['expires_at']) : translate('N/A');

				   echo htmlspecialchars($date);
				?>
			    </td>
			</tr>
			<tr>
			    <th width="20%"><?php echo translate('Refresh Token'); ?></th>

			    <td>
				<?php
				   echo htmlspecialchars(isset($token['refresh_token']) ? $token['refresh_token'] : translate('N/A'));

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


}//end class
?>
