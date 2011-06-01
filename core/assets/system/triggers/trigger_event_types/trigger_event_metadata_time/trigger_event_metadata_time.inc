<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: trigger_event_metadata_time.inc,v 1.1 2008/08/21 11:43:07 gnoel Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/system/triggers/trigger_event/trigger_event.inc';

/**
* Trigger_Event_Metadata_Time
*
* Fired by the Cron Job Metadata Trigger when a metadata field date is expired
*
* @author  Geoffroy Noel <gnoel@squiz.co.uk>
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trigger_Event_Metadata_Time extends Trigger_Event
{

}//end class

?>