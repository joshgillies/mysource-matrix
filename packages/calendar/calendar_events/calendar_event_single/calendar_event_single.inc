<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Open Source Licence                                      |
* +--------------------------------------------------------------------+
* | Copyright (c), 2003 Squiz Pty Ltd (ABN 77 084 670 600).            |
* +--------------------------------------------------------------------+
* | This source file may be used subject to, and only in accordance    |
* | with, the Squiz Open Source Licence Agreement found at             |
* | http://www.squiz.net/licence.                                      |
* | Make sure you have read and accept the terms of that licence,      |
* | including its limitations of liability and disclaimers, before     |
* | using this software in any way. Your use of this software is       |
* | deemed to constitute agreement to be bound by that licence. If you |
* | modify, adapt or enhance this software, you agree to assign your   |
* | intellectual property rights in the modification, adaptation and   |
* | enhancement to Squiz Pty Ltd for use and distribution under that   |
* | licence.                                                           |
* +--------------------------------------------------------------------+
*
* $Id: calendar_event_single.inc,v 1.3 2004/07/29 03:09:28 tbarrett Exp $
* $Name: not supported by cvs2svn $
*/

require_once  dirname(__FILE__).'/../../calendar_event/calendar_event.inc';

/**
* Calendar_Event_Single
*
* Purpose
*     
*
*
* @author  Luke Wright <lwright@squiz.net>
* @author  Tom Barrett <tbarrett@squiz.net>
* @version $Revision: 1.3 $ - 1.0
* @package MySource_Matrix_Packages
* @subpackage calendar
*/
class Calendar_Event_Single extends Calendar_Event
{

	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Calendar_Event_Single($assetid=0)
	{
		$this->Calendar_Event($assetid);

	}//end Calendar_Event_Single()


	/**
	* Create this asset
	*
	* @param Array	&$link	information used to create the initial link
	*
	* @return mixed int or boolean	the link ID or the create status if no link
	* @access public
	* @see Asset::create()
	*/
	function create(&$link) 
	{
		$parent = &$link['asset'];
		if ($parent->type() == 'calendar_event_cancellation') {
			if (!$this->attr('name')) {
				$this->setAttrValue('name', 'Rescheduled for ' . $this->getFriendlyStart());
			} else {
				$good_words = array_intersect(explode(' ', strtolower($this->attr('name'))), Array('changed', 'moved', 'rescheduled'));
				if (empty($good_words)) {
					$this->setAttrValue('name', 'Rescheduled: ' . $this->attr('name'));
				}
			}
		}
		return parent::create($link);

	}//end create()

}//end class

?>