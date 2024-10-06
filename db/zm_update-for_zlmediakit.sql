--
-- Update Monitors table to have ZlMediaKit
--

SELECT 'Checking for ZlMediaKitEnabled in Monitors';
SET @s = (SELECT IF(
  (SELECT COUNT(*)
  FROM INFORMATION_SCHEMA.COLUMNS
  WHERE table_name = 'Monitors'
  AND table_schema = DATABASE()
  AND column_name = 'ZlMediaKitEnabled'
  ) > 0,
"SELECT 'Column ZlMediaKitEnabled already exists on Monitors'",
 "ALTER TABLE `Monitors` ADD COLUMN `ZlMediaKitEnabled` BOOLEAN NOT NULL default false AFTER `Decoding`"
));

PREPARE stmt FROM @s;
EXECUTE stmt;

SELECT 'Checking for ZlMediaKitType in Monitors';
SET @s = (SELECT IF(
  (SELECT COUNT(*)
  FROM INFORMATION_SCHEMA.COLUMNS
  WHERE table_name = 'Monitors'
  AND table_schema = DATABASE()
  AND column_name = 'ZlMediaKitType'
  ) > 0,
"SELECT 'Column ZlMediaKitType already exists on Monitors'",
 "ALTER TABLE `Monitors` ADD COLUMN `ZlMediaKitType` enum('HLS','MSE','WebRTC') NOT NULL default 'WebRTC' AFTER `ZlMediaKitEnabled`"
));

PREPARE stmt FROM @s;
EXECUTE stmt;

SELECT 'Checking for ZlMediaKitAudioEnabled in Monitors';
SET @s = (SELECT IF(
  (SELECT COUNT(*)
  FROM INFORMATION_SCHEMA.COLUMNS
  WHERE table_name = 'Monitors'
  AND table_schema = DATABASE()
  AND column_name = 'ZlMediaKitAudioEnabled'
  ) > 0,
"SELECT 'Column ZlMediaKitAudioEnabled already exists in Monitors'",
"ALTER TABLE `Monitors` ADD COLUMN `ZlMediaKitAudioEnabled` BOOLEAN NOT NULL default false AFTER `ZlMediaKitType`"
));

PREPARE stmt FROM @s;
EXECUTE stmt;

INSERT INTO Config SET Id = 242, Name = 'ZM_ZLMEDIAKIT_SECRET', Value = '', Type = 'string', DefaultValue = '', Hint = 'string', Pattern = '(?^:^(.+)$)', Format = ' $1 ', Prompt = 'Password for ZlMediaKit  streaming administration.', Help = 'This value should be set to a secure password,
      and match the \"secret\" value in ZlMediaKit config.ini .
      ', Category = 'system', Readonly = '0', Requires = '';
INSERT INTO Config SET Id = 243, Name = 'ZM_ZLMEDIAKIT_PATH', Value = '', Type = 'string', DefaultValue = '', Hint = 'string', Pattern = '(?^:^(.+)$)', Format = ' $1 ', Prompt = 'URL for ZlMediaKit HTTP/S port', Help = 'ZlMediaKit requires HTTP/S communication to administer
      and initiate webrtc streams. If left blank, this will default to
      the ZM hostname, port 443/zlmediakit. This setting is particularly
      useful for putting ZlMediaKit behind a reverse proxy.
      ', Category = 'system', Readonly = '0', Requires = '';


