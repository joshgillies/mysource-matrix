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
* $Id: ipb_root_forum_edit_fns.inc,v 1.5 2012/08/30 00:58:55 ewang Exp $
*
*/

require_once SQ_PACKAGES_PATH.'/ipb/ipb_forum/ipb_forum_edit_fns.inc';

/**
* Ipb_Root_Forum_Edit_Fns
*
* Purpose
*
*
* @author  Scott Kim <skim@squiz.net>
* @version $Revision: 1.5 $
* @package MySource_Matrix_Packages
* @subpackage ipb
*/
class Ipb_Root_Forum_Edit_Fns extends Ipb_Forum_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Ipb_Root_Forum_Edit_Fns()
	{
		$this->Ipb_Forum_Edit_Fns();

	}//end constructor


	/**
	* Paint the interface for the list of root forums
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintListOfForums(&$asset, &$o, $prefix)
	{
		$ipb_bridge =& $GLOBALS['SQ_SYSTEM']->am->getAsset($asset->_ipbid);
		$ipb =& $ipb_bridge->getIpbConn();
		$tmp = explode(':', $asset->id);
		$forums = $ipb->getListOfForums($tmp[1]);

		if (empty($forums)) {
			$o->openField(translate('List of Root Forums'));

				echo translate('Currently there is no forums in IPB');

			$o->closeField();
		} else {
			$o->openField(translate('List of Forums'));

				?>
				<table class="sq-backend-table">
					<tr>
						<th style="width: 20%"><?php echo translate('Name'); ?></th>

						<th style="width: 50%"><?php echo translate('Description'); ?></th>

						<th style="width: 30%"><?php echo translate('Asset'); ?></th>

					</tr>
				<?php
				foreach ($forums as $forum) {
					$data = $ipb->getForumInfo($ipb_bridge->id.':'.'f_'.$forum['id']);
				?>
					<tr>
						<td style="font-weight: bold;"><?php echo $data['name']; ?></td>
						<td><?php echo $data['description']; ?></td>
						<td><?php echo  get_asset_tag_line($asset->_ipbid.':'.'r_'.$data['id']); ?></td>
					</tr>
				<?php
				}
			$o->closeField();
		}
		return FALSE;

	}//end paintListOfForums()


	/**
	* Paint the interface for the list of root forums
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processListOfForums(&$asset, &$o, $prefix)
	{
		return FALSE;

	}//end processListOfForums()


}//end class

?>
