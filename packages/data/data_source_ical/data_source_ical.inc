<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd	   |
* | ACN 084 670 600													   |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.														   |
* +--------------------------------------------------------------------+
*
* $Id: data_source_ical.inc,v 1.8 2011/08/11 03:41:02 cupreti Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset.inc';
require_once SQ_CORE_PACKAGE_PATH.'/data_source/data_source/data_source.inc';
require_once SQ_LIB_PATH.'/html_form/html_form.inc';
require_once SQ_FUDGE_PATH.'/general/datetime.inc';
require_once(SQ_INCLUDE_PATH.'/general_occasional.inc');

/**
* Data_Source_iCal
*
* Purpose
*
*
* @author  Benjamin Pearson <bpearson@squiz.net>
* @version $Revision: 1.8 $
* @package MySource_Matrix_Packages
* @subpackage data
*/
class Data_Source_iCal extends Data_Source
{


	/**
	* Constructor
	*
	* @param int	$assetid		assetid for the asset
	*
	*/
	function __construct($assetid=0)
	{
		parent::__construct($assetid);

	}//end constructor


	/**
	* Returns an array of all the permitted links type, the type asset and the cardinality
	* In the form
	*
	*   Array('[link_type]' => Array('[type_code]' => Array('card' => [cardinality], 'exclusive' => [exclusive])));
	* Where:
	*   link_type   = SQ_LINK_TYPE_1|SQ_LINK_TYPE_2|SQ_LINK_TYPE_3|SQ_LINK_NOTICE
	*   cardinality = [max number of links]|'M'
	*   exclusive   = TRUE|FALSE
	*
	* @return array
	* @access private
	*/
	function _getAllowedLinks()
	{
		// any link is allowed
		$allowed_link['asset']['card'] = 'M';
		$allowed_link['asset']['exclusive'] = FALSE;

		$links[SQ_LINK_TYPE_1] = $allowed_link;
		$links[SQ_LINK_TYPE_2] = $allowed_link;
		$links[SQ_LINK_TYPE_3] = $allowed_link;
		$links[SQ_LINK_NOTICE] = $allowed_link;

		return $links;

	}//end _getAllowedLinks()


	/**
	* Returns an array of the result of query execution
	*
	* @return array
	* @access public
	*/
	function &getResultSet()
	{
		$GLOBALS['SQ_SYSTEM']->pm->startTimer($this);

		$hash = md5($this->name);

		if (!isset($this->_tmp[$hash])) {
			$result = parent::getResultSet($hash);
			if (empty($result)) {
					$result = $this->getItems();
					if (!empty($result)) {
						$this->_tmp[$hash] = $result;
						parent::setResultSet($result, $hash);
					}//end if
			} else {
				$this->_tmp[$hash] = $result;
			}
		} else {
			$result = $this->_tmp[$hash];
		}

		$GLOBALS['SQ_SYSTEM']->pm->stopTimer($this);

		return $result;

	}//end getResultSet()


	/**
	* Function to read the iCal feed and return its results 
	*
	* @param string	$path	the path to the iCal feed
	*
	* @return array
	* @access public
	*/
	function getItems()
	{
		$result = Array();
		$file = $this->attr('path');
		if ((strpos($file, 'http://') === 0) || (strpos($file, 'ftp://') === 0)) {
			$file = str_replace(' ', '%20', $file);
		}//end if
		$ical = open_socket_connection($file, $errno, $errstr);
		$results = Array();
		$startCalendar = FALSE;
		$startEvent = FALSE;
		$index = 0;

		if ($ical) {
			while (!feof($ical)) {
				$line = fgets($ical, 4096);
				$line = trim($line);
				$matches = Array();
				$pattern = '/([^;:]*)([^:]*):(.*)/is';
				$status = preg_match($pattern, $line, $matches);
				$command = array_get_index($matches, 1, '');
				$parameter = array_get_index($matches, 3, '');

				if (!empty($command)) {
					switch ($command) {
						case 'BEGIN':
							if ($startCalendar) {
								// We are in the calendar, so allow us to begin events etc.
								if ($parameter == 'VEVENT') {
									$startEvent = TRUE;
									$results[$index] = Array();
								}//end if
							} else {
								// Not in calendar
								if ($parameter == 'VCALENDAR') {
									$startCalendar = TRUE;
								}//end if
							}//end if
						break;
						case 'END':
							if ($parameter == 'VEVENT') {
								$startEvent = FALSE;
								$index++;
							} else if ($parameter == 'VCALENDAR') {
								$startCalendar = TRUE;
							}//end if
						break;
						default:
							if ($startEvent) {
								switch ($command) {
									case 'DTEND':
									case 'DTSTAMP':
									case 'DTSTART':
									case 'CREATED':
									case 'LAST-MODIFIED':
										$results[$index][strtolower($command.'-raw')] = $parameter;
										$date_str = $parameter;
										if (strlen($parameter) == 8) {
											$date_str .= 'T------';
										}//end if
										if (substr($date_str, 15, 1) === 'Z') {
											$timestamp = gmmktime(	substr($date_str, 9, 2),
																	substr($date_str, 11, 2),
																	substr($date_str, 13, 2),
																	substr($date_str, 4, 2),
																	substr($date_str, 6, 2),
																	substr($date_str, 0, 4) );
											$value = readable_datetime($timestamp);
										} else {
											$date_value  = substr($date_str, 0, 4);			// Year
											$date_value .= '-'.substr($date_str, 4, 2);		// Month
											$date_value .= '-'.substr($date_str, 6, 2);		// Day
											$date_value .= 'T'.substr($date_str, 9, 2);		// Hour
											$date_value .= ':'.substr($date_str, 11, 2);	// Minute
											$date_value .= ':'.substr($date_str, 13, 2);	// Second
											$value = readable_datetime(iso8601_ts($date_value));
										}//end if
									break;
									default:
										$value = $parameter;
								}//end switch
								$results[$index][strtolower($command)] = $value;
								if ($command == 'SUMMARY') {
									$results[$index]['title'] = ellipsisize($value, 20);
								}//end if
							}//end if
					}//end switch
				}//end if
			}//end while
			fclose($ical);
		}//end if

		return $results;

	}//end getItems()


}//end class

?>
