<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix Module file is Copyright (c) Squiz Pty Ltd    |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: This Module is not available under an open source       |
* | license and consequently distribution of this and any other files  |
* | that comprise this Module is prohibited. You may only use this     |
* | Module if you have the written consent of Squiz.                   |
* +--------------------------------------------------------------------+
*
* $Id: ipb_topic.inc,v 1.4 2009/07/12 23:49:18 lwright Exp $
*
*/

check_ssv_licence();
require_once SQ_PACKAGES_PATH.'/ipb/ipb_forum/ipb_forum.inc';


/**
* Ipb_Topic
*
* Purpose
*
*
* @author  Scott Kim <skim@squiz.net>
* @version $Revision: 1.4 $
* @package MySource_Matrix_Packages
* @subpackage ipb
*/
class Ipb_Topic extends Ipb_Forum
{


	/**
	* Constructor
	*
	* @param int	$ldapid	the ID of the IPB bridge that links to the IPB for this user
	* @param array	$data	the IPB entry for this user (contains all user data)
	*
	*/
	function Ipb_Topic($ipbid=0, $data=Array())
	{
		$this->Ipb_Forum($ipbid, $data);

		if ($this->_prefix == 't') {
			$this->vars['description']['type'] = 'text';
			$this->vars['description']['value'][0] = 'IPB Topic';

			$this->vars['posts']['type'] = 'int';
			$this->vars['posts']['value'][0] = $data['posts'] + 1;

			$this->vars['starter_name']['type'] = 'text';
			$this->vars['starter_name']['value'][0] = $data['starter_name'];

			$this->vars['start_date']['type'] = 'text';
			$value =  date('g:i:s D j M Y', intval($data['start_date']));
			$this->vars['start_date']['value'][0] = $value;

			$this->vars['last_poster_name']['type'] = 'text';
			$this->vars['last_poster_name']['value'][0] = $data['last_poster_name'];

			$this->vars['last_post']['type'] = 'text';
			$value =  date('g:i:s D j M Y', intval($data['last_post']));
			$this->vars['last_post']['value'][0] = $value;
		}

	}//end constructor


}//end class

?>
