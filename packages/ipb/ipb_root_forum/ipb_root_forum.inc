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
* $Id: ipb_root_forum.inc,v 1.8 2012/08/30 00:58:55 ewang Exp $
*
*/


require_once SQ_PACKAGES_PATH.'/ipb/ipb_forum/ipb_forum.inc';


/**
* Ipb_Root_Forum
*
* Purpose
*
*
* @author  Scott Kim <skim@squiz.net>
* @version $Revision: 1.8 $
* @package MySource_Matrix_Packages
* @subpackage ipb
*/
class Ipb_Root_Forum extends Ipb_Forum
{


	/**
	* Constructor
	*
	* @param int	$ldapid	the ID of the IPB bridge that links to the IPB for this user
	* @param array	$data	the IPB entry for this user (contains all user data)
	*
	*/
	function Ipb_Root_Forum($ipbid=0, $data=Array())
	{
		$this->Ipb_Forum($ipbid, $data);

		// number of root forums
		if ($ipbid != 0) {
			$ipb_bridge =& $GLOBALS['SQ_SYSTEM']->am->getAsset($ipbid);
			$ipb =& $ipb_bridge->getIpbConn();
			$forums = $ipb->getListOfForums($data['id']);

			$this->vars['description']['type'] = 'text';
			$this->vars['description']['value'] = 'IPB Root Forum';

			$this->vars['forums']['type'] = 'int';
			$this->vars['forums']['value'] = count($forums);
		}

	}//end constructor


}//end class

?>
