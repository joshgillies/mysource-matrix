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
* $Id: ipb_post.inc,v 1.4 2007/01/08 03:49:23 emcdonald Exp $
*
*/

check_ssv_licence();
require_once SQ_PACKAGES_PATH.'/ipb/ipb_topic/ipb_topic.inc';

/**
* Ipb_Post
*
* Purpose
*
*
* @author  Scott Kim <skim@squiz.net>
* @version $Revision: 1.4 $
* @package MySource_Matrix_Packages
* @subpackage ipb
*/
class Ipb_Post extends Ipb_Topic
{


	/**
	* Constructor
	*
	* @param int	$ldapid	the ID of the IPB bridge that links to the IPB for this user
	* @param array	$data	the IPB entry for this user (contains all user data)
	*
	*/
	function Ipb_Post($ipbid=0, $data=Array())
	{
		$data['name'] = 'IPB Post';
		$this->Ipb_Topic($ipbid, $data);

		if (array_key_exists('author_name', $data)) {
			$this->vars['author_name']['type'] = 'text';
			$this->vars['author_name']['value'] = $data['author_name'];
		}

		if (array_key_exists('post_date', $data)) {
			$this->vars['post_date']['type'] = 'text';
			$value =  date('g:i:s D j M Y', intval($data['post_date']));
			$this->vars['post_date']['value'] = $value;
		}

		if (array_key_exists('post_title', $data)) {
			$this->vars['post_title']['type'] = 'text';
			$this->vars['post_title']['value'] = (empty($data['post_title'])) ? 'none' : $data['post_title'];
		}

		if (array_key_exists('post_parent', $data)) {
			$this->vars['post_parent']['type'] = 'text';
			$this->vars['post_parent']['value'] = ($data['post_parent']) ? 'Yes' : 'No';
		}

		if (array_key_exists('post', $data)) {
			$this->vars['post']['type'] = 'text';
			$this->vars['post']['value'] = $data['post'];
		}

		if (array_key_exists('topic_name', $data)) {
			$this->vars['topic_name']['type'] = 'text';
			$this->vars['topic_name']['value'] = $data['topic_name'];
		}

		if (array_key_exists('forum_name', $data)) {
			$this->vars['forum_name']['type'] = 'text';
			$this->vars['forum_name']['value'] = $data['forum_name'];
		}

		if (array_key_exists('forum_name', $data)) {
			$time = date('D jS M h:ia', intval($data['post_date']));
			$this->name = $time.' by '.$data['author_name'].' in '.$data['forum_name'];
			$this->short_name = $this->name;
		}

	}//end constructor


}//end class

?>
