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
* $Id: page_calendar_management.inc,v 1.3 2004/08/04 23:39:07 lwright Exp $
* $Name: not supported by cvs2svn $
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Page_Calendar_Management
*
*
* @author  Luke Wright <lwright@squiz.net>
* @version $Revision: 1.3 $ - 1.0
* @package MySource_Matrix_Packages
* @subpackage calendar
*/
class Page_Calendar_Management extends Asset_Management
{


	/**
	* Constructor
	*
	* @param object Package_Manager	&$pm	Reference to the package manager object for this package
	*
	*/
	function Page_Calendar_Management(&$pm)
	{
		$this->Asset_Management($pm);

		// root nodes will be held in an array indexed by asset id and containing
		// data such as CSS styles and other stuff (structure not yet determined)
		$this->vars['root_nodes']			= Array(
														'added'     => '0.1',
														'type'      => 'serialise',
														'default'   => Array(),
														);
		
		// spacing of the day view calendar
		// (should this be enforced to be a factor of 60, like 60 30 20 15 10 6 5?)
		$this->vars['appointment_spacing']	= Array(
														'added'     => '0.1',
														'type'      => 'int',
														'default'   => 30,
														);
		
		// These are stored as strings, 'hh:mm' because I want '24:00' to represent the
		// latest possible end time (ie. end of the day) but that will be rejected by a
		// datetime field as out of the range
		$this->vars['day_view_start_time']	= Array(
														'added'     => '0.1',
														'type'      => 'text',
														'default'   => '00:00',
														);
		
		$this->vars['day_view_end_time']	= Array(
														'added'     => '0.1',
														'type'      => 'text',
														'default'   => '24:00',
														);
														
		// currently applies to both week and month views
		$this->vars['week_starts_on']		= Array(
														'added'     => '0.1',
														'type'      => 'int',
														'default'   => 1,	// Monday
														);
														
		// currently applies to week view, to display a selective day view
		// defaults to working week (Monday..Friday)
		$this->vars['week_view_show_days']			= Array(
														'added'     => '0.1',
														'type'      => 'serialise',
														'default'   => range(1,5),	//Mo-Fr
														);
		
		
	}//end Page_Calendar_Management()


}//end class

?>