<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ABN 77 084 670 600                                                 |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: condition_user_ip_edit_fns.inc,v 1.16 2012/08/30 01:09:10 ewang Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/system/conditions/condition/condition_edit_fns.inc';
require_once SQ_FUDGE_PATH.'/csv/csv.inc';
require_once dirname(__FILE__).'/condition_user_ip.inc';


/**
* Condition_User_IP_Edit_Fns
*
* Edit functions for the user IP condition
*
* @see Asset_Edit_Fns
*
* @author  Luke Wright <lwright@squiz.net>
* @author  David Heppell <dheppell@squiz.net>
* @version $Revision: 1.16 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Condition_User_IP_Edit_Fns extends Condition_Edit_Fns
{


	/**
	* Constructor
	*
	* @see Asset_Edit_Fns::__construct()
	*/
	function __construct()
	{
		parent::__construct();

	}//end constructor


	/**
	* Paint the backend editing interface for this condition
	*
	* @param array				&$condition_data	Array of settings for this condition
	* @param Backend_Outputter	$o					Backend outputter to use
	* @param string				$prefix				Prefix for form elements
	* @param boolean			$write_access		Whether we have access to edit the field values
	*
	* @return void
	* @access public
	*/
	public static function paintEditInterface(Array $condition_data, Backend_Outputter $o, $prefix, $write_access)
	{
		if (!isset($condition_data['user_ip_ranges'])) {
			$condition_data['user_ip_ranges'] = Array();
		}

		if (!isset($condition_data['default_grant'])) {
			$condition_data['default_grant'] = FALSE;
		}

		if ($write_access) {
			$condition_data['user_ip_ranges']['new'] = Array('ip_address' => NULL, 'subnet' => NULL, 'grant' => 1);
		}

		$grant_options = Array(1 => translate('Grant'), 0 => translate('Deny'));



		if (!$write_access && empty($condition_data['user_ip_ranges'])) {
			?><em><?php echo translate('There are no conditions set'); ?></em><?php

		} else {
			?>
			<table class="sq-backend-table">
				<tr>
				<?php if ($write_access) {
				?>
					<th></th>
				<?php
				}
				?>
				<th><?php echo translate('Network IP Address'); ?></th>

				<th></th>
				<th><?php echo translate('Subnet Mask'); ?></th>

				<th><?php echo translate('Action'); ?></th>

				<?php if ($write_access) {
				?>
					<th><?php echo translate('Delete?'); ?></th>

				<?php
				}
				?>
				</tr>
				<?php foreach ($condition_data['user_ip_ranges'] as $index => $ranges) {
				?>
					<tr>
						<?php if ($write_access) {
						?>
						<td>
							<?php
							if ($index === 'new') {
								echo '<strong>'.translate('New?').'</strong>';

							}
							?>
						</td>
						<?php
						}
						?>
						<td>
							<?php
							if ($write_access) {
								self::paintIPAddressField($prefix.'['.$index.'][ip_address]', $ranges['ip_address']);
							} else {
								echo $ranges['ip_address'];
							}
							?>
						</td>
						<td>
						/
						</td>
						<td>
							<?php
							if ($write_access) {
								self::paintSubnetMaskField($prefix.'['.$index.'][subnet]', $ranges['subnet']);
							} else {
								echo $ranges['subnet'];
							}
							?>
						</td>
						<td>
						<?php
							if ($write_access) {
								?><?php
									combo_box($prefix.'['.$index.'][grant]', $grant_options, FALSE, Array($ranges['grant']));
								?><?php
							} else {
								?><strong style="color: <?php echo $ranges['grant'] ? 'green' : 'red' ?>"><?php
								echo $grant_options[$ranges['grant']];
								?></strong><?php
							}
						?></td>
						<td style="text-align: center">
						<?php
							if ($write_access && ($index !== 'new')) {
								check_box($prefix.'['.$index.'][delete]', FALSE);
							} else {
								echo '&nbsp;';
							}
						?></td>
					</tr>
				<?php
				}//end foreach
				?>
			</table>
		<?php
		}//end else

		echo '<p>';
		echo translate('If IP address is outside one of these ranges').': ';

		if ($write_access) {
			combo_box($prefix.'[default_grant]', $grant_options, FALSE, Array($condition_data['default_grant']));
		} else {
			?><strong style="color: <?php echo $condition_data['default_grant'] ? 'green' : 'red' ?>"><?php
			echo $grant_options[$condition_data['default_grant']];
			?></strong><?php

		}
		echo '</p>';

		echo '<p>';
		echo '<b>'.translate('Import rules from CSV file').':</b>&nbsp;';

		file_upload($prefix.'_import');
		echo '</p>';
		$o->note(translate('The file uploaded should be a CSV file without a header row. Each line should contain exactly six fields: the first four being the octets of the network IP address, the fifth being the subnet mask (in CIDR format, eg. "255.255.255.0" would be represented in the file by "24"), and the sixth being a one (1) for a Grant rule or zero (0) for a Deny rule. The default Grant/Deny setting is not affected by any upload and should be set separately.'));


	}//end paintEditInterface()


	/**
	* Process the backend editing interface for this condition
	*
	* @param Backend_Outputter	$o		Backend outputter to use
	* @param string				$prefix	Prefix for form elements
	*
	* @return array
	* @access public
	*/
	public static function processEditInterface(Backend_Outputter $o, $prefix)
	{
		if (isset($_FILES[$prefix.'_import']) && ($_FILES[$prefix.'_import']['error'] == 0)) {
			$file_name = $_FILES[$prefix.'_import']['tmp_name'];
			if (is_uploaded_file($file_name)) {
				$condition_data = self::importRulesFromCSV($file_name);
				if ($condition_data !== FALSE) {
					$condition_data['default_grant'] = $_POST[$prefix]['default_grant'];
					return $condition_data;
				} else {
					trigger_localised_error('CORE0249', translate('Ignoring uploaded IP ranges file as it appears to be invalid'), E_USER_WARNING);
				}
			}
		}

		// construct and return data required by this condition
		// as found in the post data, array required consists of
		// 'match' and 'condition_data'
		$ip_ranges = Array();

		foreach ($_POST[$prefix] as $index => $post_data) {
			if (!isset($post_data['delete']) && is_array($post_data)) {
				$this_row = Array(
								'ip_address'	=> self::processIPAddressField($post_data['ip_address']),
								'subnet'		=> self::processSubnetMaskField($post_data['subnet']),
								'grant'			=> $post_data['grant'],
							);

				// IP address not filled in => continue with life
				if (empty($this_row['ip_address'])) continue;

				// see if there is an exact duplicate, skip it if there is
				if (in_array($this_row, $ip_ranges)) continue;

				// now flip the grant field to see if there is a IP/subnet dupe
				// of the other kind of rule
				$this_row['grant'] = !$this_row['grant'];
				if (in_array($this_row, $ip_ranges)) {
					trigger_localised_error('CORE0250', sprintf(translate('The IP/subnet combination "%1$s/%2$s" is in the file once for both grant and deny - using the first line found and ignoring the rest'), $this_row['ip_address'], $this_row['subnet']), E_USER_WARNING);
					continue;
				}

				// Check to see whether the ip address is a valid network address
				// for the subnet mask in question
				if (self::_doesIPBeginSubnet($this_row['ip_address'], $this_row['subnet'])) {
					// put grant back to where it was before and store it
					$this_row['grant'] = $post_data['grant'];
					$ip_ranges[] = $this_row;
				} else {
					trigger_localised_error('CORE0248', sprintf(translate('The IP/subnet combination "%1$s/%2$s" does not represent a valid network IP - ignoring'), $this_row['ip_address'], $this_row['subnet']), E_USER_WARNING);
				}
			}//end if
		}//end foreach

		$results = Array(
					'user_ip_ranges'	=> $ip_ranges,
					'default_grant'		=> $_POST[$prefix]['default_grant'],
				   );

		return $results;

	}//end processEditInterface()


	/**
	* Paints an IP address field
	*
	* @param string	$prefix	the prefix for form fields
	* @param string	$value	the current value of the subnet mask (in dot notation)
	*						(default: blank fields)
	*
	* @return void
	* @access public?
	*/
	public static function paintIPAddressField($prefix, $value=NULL)
	{
		if ($value == NULL) $value = '...';
		$value = explode('.', $value);

		text_box($prefix.'[0]', $value[0], 3, 0, 3);
		echo '.';
		text_box($prefix.'[1]', $value[1], 3, 0, 3);
		echo '.';
		text_box($prefix.'[2]', $value[2], 3, 0, 3);
		echo '.';
		text_box($prefix.'[3]', $value[3], 3, 0, 3);

	}//end paintIPAddressField()


	/**
	* Processes an IP address field
	*
	* @param array	$octets	the octets from the form response
	*
	* @return mixed int|boolean
	* @access public?
	*/
	public static function processIPAddressField($octets)
	{
		// if there is a blank in any field - reject the IP address
		for ($i = 0; $i <= 3; $i++) {
			if ($octets[$i] == '') return FALSE;
		}

		// non-integer bulldust filter
		$octets[0] = (int)$octets[0];
		$octets[1] = (int)$octets[1];
		$octets[2] = (int)$octets[2];
		$octets[3] = (int)$octets[3];

		for ($i = 0; $i <= 3; $i++) {
			if (($octets[$i] < 0) || ($octets[$i] > 255)) {
				return FALSE;
			}
		}

		return implode('.', $octets);

	}//end processIPAddressField()


	/**
	* Paints a subnet mask field
	*
	* @param string	$prefix	the prefix for form fields
	* @param string	$value	the current value of the subnet mask (in dot notation)
	*						(default: '255.255.255.255' - single IP)
	*
	* @return void
	* @access public?
	*/
	public static function paintSubnetMaskField($prefix, $value=NULL)
	{
		// Save all the possible bitmasks so we can use them in a drop-down
		$possible_bitmasks = self::_getPossibleSubnetOctets();

		if ($value == NULL) $value = '255.255.255.255';
		$value = explode('.', $value);

		combo_box($prefix.'[0]', $possible_bitmasks, FALSE, Array($value[0]));
		echo '.';
		combo_box($prefix.'[1]', $possible_bitmasks, FALSE, Array($value[1]));
		echo '.';
		combo_box($prefix.'[2]', $possible_bitmasks, FALSE, Array($value[2]));
		echo '.';
		combo_box($prefix.'[3]', $possible_bitmasks, FALSE, Array($value[3]));

	}//end paintSubnetMaskField()


	/**
	* Processes a subnet mask field
	*
	* @param array	$octets	the data from the form request
	*
	* @return mixed string|NULL
	* @access public?
	*/
	public static function processSubnetMaskField($octets)
	{
		// non-integer bulldust filter
		$octets[0] = (int)$octets[0];
		$octets[1] = (int)$octets[1];
		$octets[2] = (int)$octets[2];
		$octets[3] = (int)$octets[3];

		// Make sure we don't have a weird bitmask like '255.255.128.224' -
		// once we've got a ZERO bit the remaining bits must be ZERO also
		$ip_address = implode('.', $octets);

		if (self::_isValidSubnetMask($ip_address)) {
			return $ip_address;
		} else {
			trigger_error('Subnet mask entered is not valid', E_USER_WARNING);
			return NULL;
		}

	}//end processSubnetMaskField()


	/**
	* Converts a Subnet IP mask to a Classless Inter-Domain Routing (CIDR) bit-count
	* or FALSE if IP address is an invalid subnet mask
	*
	* For example:
	* - given the subnet mask '255.255.255.255', this function returns 32
	* - given the subnet mask '255.248.0.0', this function returns 13
	* - given the subnet mask '255.248.0.255', this function returns FALSE
	*
	* @param string	$subnet_ip	The subnet mask in IP address format
	*
	* @return mixed int|boolean
	* @access public?
	*/
	public static function convertSubnetIPtoCIDR($subnet_ip)
	{
		if (!self::_isValidSubnetMask($subnet_ip)) {
			return FALSE;
		}

		$octets = explode('.', $subnet_ip);
		$cidr_value = 0;

		for ($i = 0; $i <= 3; $i++) {
			if ($octets[$i] == 0) break;
			while ($octets[$i] > 0) {
				if ($octets[$i] % 2 == 1) $cidr_value++;
				$octets[$i] >>= 1;
			}
		}

		return $cidr_value;

	}//end convertSubnetIPtoCIDR()


	/**
	* Converts a CIDR bit-count to a subnet IP mask
	* or FALSE if CIDR value is invalid (outside the range 0..32)
	*
	* For example:
	* - given a CIDR value of 24, this function should return '255.255.255.0'
	* - given a CIDR value of 33, this function should return FALSE
	*
	* @param int	$cidr_value	the CIDR bit-count value
	*
	* @return mixed string|boolean
	* @access public?
	*/
	public static function convertCIDRtoSubnetIP($cidr_value)
	{
		$cidr_value = (int)$cidr_value;
		if (($cidr_value < 0) || ($cidr_value > 32)) {
			return FALSE;
		}

		$octets = Array(0, 0, 0, 0);
		$current_octet = 0;

		while ($cidr_value > 0) {
			if ($cidr_value > 8) {
				// might as well get rid of the 255's as a whole block
				$cidr_value -= 8;
				$octets[$current_octet] = 255;
				$current_octet++;
			} else {
				// not enough bits left for a 255?
				for ($i = 0; $i < 8; $i++) {
					$octets[$current_octet] <<= 1;
					if ($cidr_value > 0) {
						$octets[$current_octet]++;
						$cidr_value--;
					}
				}
			}
		}

		return implode('.', $octets);

	}//end convertCIDRtoSubnetIP()


	/**
	* Returns TRUE if the IP address is a part of the subnet specified
	*
	* Remote user's IP is in the subnet if you bitwise-AND it with the subnet
	* mask and it comes up with the same IP address as the passed network IP.
	*
	* @param string	$ip_address	the IP address of the remote user
	* @param string	$network_ip	the network IP address to test against
	* @param string	$subnet_ip	the subnet mask to use
	*
	* @return boolean
	* @access public
	*/
	public static function isInSubnet($ip_address, $network_ip, $subnet_ip)
	{
		$ip_octets = explode('.', $ip_address);
		$network_octets = explode('.', $network_ip);
		$subnet_octets = explode('.', $subnet_ip);

		for ($i = 0; $i <= 3; $i++) {
			$masked_octet = (int)$ip_octets[$i] & (int)$subnet_octets[$i];
			if ($masked_octet != (int)$network_octets[$i]) {
				return FALSE;
			}
		}

		return TRUE;

	}//end isInSubnet()


	/**
	* Returns TRUE if the IP address passed is a valid subnet mask
	*
	* @param string	$subnet_ip	the subnet mask to check
	*
	* @return boolean
	* @access private
	*/
	protected static function _isValidSubnetMask($subnet_ip)
	{
		$bitmasks = self::_getPossibleSubnetOctets();

		$octets = explode('.', $subnet_ip);
		$zero_bit_found = FALSE;

		for ($i = 0; $i <= 3; $i++) {
			if ($zero_bit_found && ($octets[$i] != 0)) {
				// If we found a zero bit in a previous octet, the rest of
				// the subnet mask must be ZERO
				return FALSE;
			} else if ($octets[$i] != 255) {
				// We've found a zero bit here - keep note of it
				$zero_bit_found = TRUE;
			}

			// If the octet is not a valid subnet octet, this IP fails it
			if (!isset($bitmasks[$octets[$i]]))	return FALSE;
		}

		return TRUE;

	}//end _isValidSubnetMask()


	/**
	* Sort IP range list in order from largest subnet to smallest subnet
	*
	* @param array	$ip_ranges	unsorted list of IP addresses
	*
	* @return array
	* @access public
	*/
	public static function sortByCIDROrder($ip_ranges)
	{
		$ip_ranges_by_cidr = Array();

		foreach ($ip_ranges as $ip_range) {
			$cidr_value = self::convertSubnetIPtoCIDR($ip_range['subnet']);
			$ip_ranges_by_cidr[$cidr_value][] = $ip_range;
		}

		ksort($ip_ranges_by_cidr);

		$ip_ranges = Array();
		foreach ($ip_ranges_by_cidr as $cidr_ip_range) {
			$ip_ranges = array_merge($ip_ranges, $cidr_ip_range);
		}

		return $ip_ranges;

	}//end sortByCIDROrder()


	/**
	* Returns a list of possible octets
	*
	* @return array
	* @access private
	*/
	protected static function _getPossibleSubnetOctets()
	{
		$bitmasks = Array(
						0	=> '0',
						128	=> '128',
						192	=> '192',
						224	=> '224',
						240	=> '240',
						248	=> '248',
						252	=> '252',
						254	=> '254',
						255	=> '255',
					);
		return $bitmasks;

	}//end _getPossibleSubnetOctets()


	/**
	* Returns TRUE if the IP address given begins the passed subnet mask
	*
	* If IP & (~Subnet) == '0.0.0.0', the IP address is the first (network)
	* address in the subnet
	*
	* @param string	$ip_address	the IP address
	* @param string	$subnet_ip	the subnet mask in IP format
	*
	* @return boolean
	* @access private
	*/
	protected static function _doesIPBeginSubnet($ip_address, $subnet_ip)
	{
		$ip_octets = explode('.', $ip_address);
		$subnet_octets = explode('.', $subnet_ip);

		for ($i = 0; $i <= 3; $i++) {
			if (($ip_octets[$i] & (~(int)$subnet_octets[$i])) != 0) {
				return FALSE;
			}
		}

		return TRUE;

	}//end _doesIPBeginSubnet()


	/**
	* import rules from a CSV file - returns array of condition data
	* or FALSE if CSV file is invalid
	*
	* @param string	$file_name	the (temporary) file name to import the CSV from
	*
	* @return mixed array|boolean
	* @access public
	*/
	public static function importRulesFromCSV($file_name)
	{
		// minimum and maximum values allowed in each field
		// CSV format: first 4 fields = IP octets, fifth = subnet CIDR, sixth = grant/deny
		$min_values = Array(0, 0, 0, 0, 1, 0);
		$max_values = Array(255, 255, 255, 255, 32, 1);

		$condition_data = Array('user_ip_ranges' => Array());
		$condition_data['default_grant'] = 0;

		$csv = new CSV();
		$csv->setFilepath($file_name);
		$csv->import();
		$lines = $csv->values;

		// where we will be building our IP ranges
		$ip_ranges = Array();

		foreach ($lines as $line_fields) {
			// doesn't have correct number of fields...
			if (count($line_fields) != 6) return FALSE;

			foreach ($line_fields as $index => $value) {
				if (!(is_numeric($value) && ($value >= $min_values[$index]) && ($value <= $max_values[$index]))) {
					return FALSE;
				}
			}

			$ip_address = implode('.', array_slice($line_fields, 0, 4));
			$subnet_mask = self::convertCIDRtoSubnetIP($line_fields[4]);
			$grant = $line_fields[5];

			$ip_range_field = Array(
								'ip_address'	=> $ip_address,
								'subnet'		=> $subnet_mask,
								'grant'			=> $grant,
							  );

			// this is already in the file - ignore it
			if (in_array($ip_range_field, $ip_ranges)) continue;

			// this has already been seen with a different grant!!
			// complain, then ignore it
			$ip_range_field['grant'] = !$ip_range_field['grant'];
			if (in_array($ip_range_field, $ip_ranges)) {
				trigger_localised_error('CORE0250', sprintf(translate('The IP/subnet combination "%1$s/%2$s" is in the file once for both grant and deny - using the first line found and ignoring the rest'), $ip_range_field['ip_address'], $ip_range_field['subnet']), E_USER_WARNING);
				continue;
			}

			// is it a valid combination?
			if (!self::_doesIPBeginSubnet($ip_range_field['ip_address'], $ip_range_field['subnet'])) {
				trigger_localised_error('CORE0248', sprintf(translate('The IP/subnet combination "%1$s/%2$s" does not represent a valid network IP - ignoring'), $ip_range_field['ip_address'], $ip_range_field['subnet']), E_USER_WARNING);
				continue;
			}

			// flip the grant back to what it was, and add it to the list
			$ip_range_field['grant'] = !$ip_range_field['grant'];
			$ip_ranges[] = $ip_range_field;
		}//end foreach

		$condition_data['user_ip_ranges'] = $ip_ranges;

		return $condition_data;

	}//end importRulesFromCSV()


}//end class

?>
