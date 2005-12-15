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
* $Id: ipb_root_forum.inc,v 1.1.1.1 2005/12/15 03:02:07 skim Exp $
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
* @version $Revision: 1.1.1.1 $
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
