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
* $Id: ipb_bridge_edit_fns.inc,v 1.8 2012/08/30 00:58:55 ewang Exp $
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
* @version $Revision: 1.8 $
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
		$version_selected = ($asset->attr('version') != 'none');
		$location = $asset->attr('location');
		$location_entered = !empty($location);

		if ($version_selected && $location_entered) {
			$ipb =& $asset->getIpbConn();
			if (!is_null($ipb) && $ipb->isConnected()) {
				echo '<span style="color: green"><b>'.translate('ipb_connected').'</b></span>';
			} else {
				echo '<span style="color: red"><b>'.translate('ipb_unable_to_connect').'</b></span>';
			}
		} else {
			echo '<span style="color: red"><b>'.translate('ipb_setup_note').'</b></span>';
		}
		return FALSE;

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
		return FALSE;

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
		$ipb =& $asset->getIpbConn();

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
		return FALSE;

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
		return FALSE;

	}//end processGoToIpb()


	/**
	* Paint the interface for editing for location
	*
	* @param object	&$asset	Ipb_Bridge the asset whose interface we are painting
	* @param object	&$o		Backend_Outputte the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintGetLocation(&$asset, &$o, $prefix)
	{
		$wa = $asset->writeAccess('attributes');
		$location = $asset->attr('location');
		if ($wa) {
			$o->openField('Location');
				text_box($prefix.'_location', $location, '100', '100');
				echo '<br />The full path of Invision Power Board';
			$o->closeField();
		} else {
			$o->openField('Location');
				if (empty($location)) {
					echo 'none';
				} else {
					echo $location;
				}
			$o->closeField();
		}
		return FALSE;

	}//end paintGetLocation()


	/**
	* Process the interface for editing for location
	*
	* @param object	&$asset	Ipb_Bridge the asset whose interface we are painting
	* @param object	&$o		Backend_Outputte the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processGetLocation(&$asset, &$o, $prefix)
	{
		$wa = $asset->writeAccess('attributes');

		if ($wa) {
			$location = trim($_POST[$prefix.'_location']);
			if (!empty($location)) {

				if ($asset->attr('version') == '3_0_2') {
					$needed_files = Array(
										'/conf_global.php',
										'/ips_kernel/classDbMysql.php',
										'/admin/sources/base/ipsRegistry.php',
									);
				} else {
					$needed_files = Array(
										'/conf_global.php',
										'/ips_kernel/class_db_mysql.php',
										'/ips_kernel/class_converge.php',
									);
				}//end if

				if (is_dir($location)) {
					foreach ($needed_files as $file) {
						if (!is_file($location.$file)) {
							trigger_localised_error('IPB0002', E_USER_WARNING, $location.$file);
							$asset->setAttrValue('location', '');
							return TRUE;
						}
					}
					$asset->setAttrValue('location', $location);
					return TRUE;
				} else {
					trigger_localised_error('IPB0001', E_USER_WARNING, $location);
				}

			}

		}//end if write access

		return FALSE;

	}//end processGetLocation()


}//end class

?>
