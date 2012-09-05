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
* $Id: page_rest_resource_oauth_session_edit_fns.inc,v 1.5 2012/09/05 02:59:45 ewang Exp $
*
*/

require_once SQ_PACKAGES_PATH.'/web_services/rest/page_templates/page_rest_resource/page_rest_resource_edit_fns.inc';

/**
* Page_REST_Resource_OAuth_Session_Edit_Fns
*
* Purpose
*
*
* @author  Maki Maruyama <mmaruyama@squiz.co.uk>
* @version $ $
* @package Squiz_UK_Clients
* @subpackage rest
*/
class Page_REST_Resource_OAuth_Session_Edit_Fns extends Page_REST_Resource_Edit_Fns
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
		$type = 'stored';
		$access_token = $asset->attr('access_token');
		$access_token_secret = $asset->attr('access_token_secret');
		if(empty($access_token) ){
		    if(isset($_SESSION['oauth'][$asset->id]['access_token'] ) && isset($_SESSION['oauth'][$asset->id]['access_token_secret'])) {
			$type = 'session';
			$access_token = $_SESSION['oauth'][$asset->id]['access_token'];
			$access_token_secret = $_SESSION['oauth'][$asset->id]['access_token_secret'];
		    }
		    else {
			$type = '';
			$access_token = 'none';
			$access_token_secret = 'none';
		    }
		}

		?>
		<table border="0" class="sq-backend-table">
		    	<tr>
			    <th width="20%"><?php echo translate('oauth_access_token_type', $type); ?></th>
			    <td>
				<?php
				   echo htmlspecialchars($type);
				?>
			    </td>
			</tr>
			<tr>
			    <th width="20%"><?php echo translate('oauth_access_token'); ?></th>
			    <td>
				<?php
				   echo htmlspecialchars($access_token);
				?>
			    </td>
			</tr>
			<tr>
			    <th width="20%"><?php echo translate('oauth_access_token_secret'); ?></th>
			    <td>
				<?php
				   echo htmlspecialchars($access_token_secret);
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
			$store_token = $asset->attr('store_token');
			if(!$store_token) {
			    $asset->setAttrValue('access_token', '');
			    $asset->setAttrValue('access_token_secret', '');
			}
			else if(isset($_SESSION['oauth'][$asset->id]['access_token'] ) && isset($_SESSION['oauth'][$asset->id]['access_token_secret'])) {
			    $asset->setAttrValue('access_token', $_SESSION['oauth'][$asset->id]['access_token']);
			    $asset->setAttrValue('access_token_secret', $_SESSION['oauth'][$asset->id]['access_token_secret']);
			}
		}

	}//end processAccessToken()
	
}//end class
?>
