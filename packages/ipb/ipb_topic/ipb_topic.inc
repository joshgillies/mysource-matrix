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
* $Id: ipb_topic.inc,v 1.8 2012/08/30 00:58:55 ewang Exp $
*
*/


require_once SQ_PACKAGES_PATH.'/ipb/ipb_forum/ipb_forum.inc';


/**
* Ipb_Topic
*
* Purpose
*
*
* @author  Scott Kim <skim@squiz.net>
* @version $Revision: 1.8 $
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
			$this->vars['description']['value'] = 'IPB Topic';

			$this->vars['posts']['type'] = 'int';
			$this->vars['posts']['value'] = $data['posts'] + 1;

			$this->vars['starter_name']['type'] = 'text';
			$this->vars['starter_name']['value'] = $data['starter_name'];

			$this->vars['start_date']['type'] = 'text';
			$value =  date('g:i:s D j M Y', intval($data['start_date']));
			$this->vars['start_date']['value'] = $value;

			$this->vars['last_poster_name']['type'] = 'text';
			$this->vars['last_poster_name']['value'] = $data['last_poster_name'];

			$this->vars['last_post']['type'] = 'text';
			$value =  date('g:i:s D j M Y', intval($data['last_post']));
			$this->vars['last_post']['value'] = $value;
		}

	}//end constructor


}//end class

?>
