<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Commercial Module Licence                                |
* +--------------------------------------------------------------------+
* | Copyright (c) Squiz Pty Ltd (ACN 084 670 600).                     |
* +--------------------------------------------------------------------+
* | This source file is not open source or freely usable and may be    |
* | used subject to, and only in accordance with, the Squiz Commercial |
* | Module Licence.                                                    |
* | Please refer to http://www.squiz.net/licence for more information. |
* +--------------------------------------------------------------------+
*
* $Id: ipb_bridge_edit_fns.inc,v 1.1.1.1 2005/12/15 03:02:07 skim Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';

/**
* IPB_Bridge_Edit_Fns
*
* Purpose
*
*
* @author  Scott Kim <skim@squiz.net>
* @version $Revision: 1.1.1.1 $
* @package MySource_Matrix_Packages
* @subpackage ipb
*/
class IPB_Bridge_Edit_Fns extends Asset_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function IPB_Bridge_Edit_Fns()
	{
		$this->Asset_Edit_Fns();

	}//end constructor


	/**
	* Paint the interface for editing user details
	*
	* @param object	&$asset	Ipb_Bridge the asset whose interface we are painting
	* @param object	&$o		Backend_Outputte the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintIpbTest(&$asset, &$o, $prefix)
	{
		$ipb = &$asset->getIpbConn();
		if (!is_null($ipb) && $ipb->isConnected()) {
			echo '<span style="color: green"><b>'.translate('ipb_connected').'</b></span>';
		} else {
			echo '<span style="color: red"><b>'.translate('ipb_unable_to_connect').'</b></span>';
		}
		$link = $GLOBALS['SQ_SYSTEM']->am->getLinks('42:p_160', SQ_LINK_TYPE_1, '', true, 'minor');
		return false;

	}//end paintIpbTest()


	/**
	* Process the interface for editing user details
	*
	* @param object	&$asset	Ipb_Bridge the asset whose interface we are painting
	* @param object	&$o		Backend_Outputte the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processIpbTest(&$asset, &$o, $prefix)
	{
		return false;

	}//end processIpbTest()


	/**
	* Paint the javascript to redirect to IPB interface
	*
	* @param object	&$asset	Ipb_Bridge the asset whose interface we are painting
	* @param object	&$o		Backend_Outputte the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintGoToIpb(&$asset, &$o, $prefix)
	{
		$ipb = &$asset->getIpbConn();

		if (!is_null($ipb) && $ipb->isConnected()) {
			$url = $ipb->getIpbURL();
			if (empty($url)) {
				$o->openSection(translate('warning'));
					$o->openField(translate('url'));
						echo translate('ipb_site_url_not_set');
					$o->closeField();
				$o->closeSection();
			} else {
			?>
				<script language="JavaScript">
					window.location="<?php echo $ipb->getIpbURL(); ?>";
				</script>
			<?php
			}
		} else {
			$o->openSection(translate('warning'));
				$o->openField(translate('connection'));
					echo translate('failed');
				$o->closeField();
			$o->closeSection();
		}

		return false;

	}//end paintGoToIpb()


	/**
	* Process the javascript to redirect to IPB interface
	*
	* @param object	&$asset	Ipb_Bridge the asset whose interface we are painting
	* @param object	&$o		Backend_Outputte the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processGoToIpb(&$asset, &$o, $prefix)
	{
		return false;

	}//end processGoToIpb()


}//end class

?>
