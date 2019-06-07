<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="ru_RU">
<context>
    <name>CrModbusPlugin</name>
    <message>
        <location filename="plugin/crmodbusplugin.cpp" line="42"/>
        <location filename="plugin/crmodbusplugin.cpp" line="43"/>
        <source>Modbus RTU client</source>
        <translation>Modbus RTU клиент</translation>
    </message>
    <message>
        <location filename="plugin/crmodbusplugin.cpp" line="51"/>
        <source>Modbus RTU client error.</source>
        <translation>Modbus RTU ошибка клиента.</translation>
    </message>
    <message>
        <location filename="plugin/crmodbusplugin.cpp" line="57"/>
        <source>Serial port</source>
        <oldsource>Selial port</oldsource>
        <translation>COM порт</translation>
    </message>
    <message>
        <location filename="plugin/crmodbusplugin.cpp" line="63"/>
        <source>Delay of frame</source>
        <translation>Перерев между кадрами</translation>
    </message>
    <message>
        <location filename="plugin/crmodbusplugin.cpp" line="74"/>
        <location filename="plugin/crmodbusplugin.cpp" line="256"/>
        <source>Enable</source>
        <translation>Включить</translation>
    </message>
    <message>
        <location filename="plugin/crmodbusplugin.cpp" line="110"/>
        <source>Modbus/RTU Client</source>
        <translation>Modbus/RTU Клиент</translation>
    </message>
    <message>
        <location filename="plugin/crmodbusplugin.cpp" line="111"/>
        <source>RTU Client</source>
        <translation>RTU Клиент</translation>
    </message>
    <message>
        <location filename="plugin/crmodbusplugin.cpp" line="119"/>
        <location filename="plugin/crmodbusplugin.cpp" line="120"/>
        <source>Modbus request</source>
        <translation>Modbus Запрос</translation>
    </message>
    <message>
        <location filename="plugin/crmodbusplugin.cpp" line="128"/>
        <source>Modbus request error.</source>
        <translation>Modbus ошибка запроса.</translation>
    </message>
    <message>
        <location filename="plugin/crmodbusplugin.cpp" line="132"/>
        <source>Data</source>
        <translation>Данные</translation>
    </message>
    <message>
        <location filename="plugin/crmodbusplugin.cpp" line="142"/>
        <source>Server address</source>
        <translation>Адрес сервера</translation>
    </message>
    <message>
        <location filename="plugin/crmodbusplugin.cpp" line="153"/>
        <source>Function code</source>
        <translation>Код функции</translation>
    </message>
    <message>
        <location filename="plugin/crmodbusplugin.cpp" line="194"/>
        <source>Start address</source>
        <translation>Начальный адрес</translation>
    </message>
    <message>
        <location filename="plugin/crmodbusplugin.cpp" line="205"/>
        <source>Size data</source>
        <translation>Размер данных</translation>
    </message>
    <message>
        <location filename="plugin/crmodbusplugin.cpp" line="215"/>
        <source>Frequency</source>
        <translation>Частота</translation>
    </message>
    <message>
        <location filename="plugin/crmodbusplugin.cpp" line="218"/>
        <source>Hz</source>
        <translation>Гц</translation>
    </message>
    <message>
        <location filename="plugin/crmodbusplugin.cpp" line="232"/>
        <source>Timeout</source>
        <translation>Таймаут</translation>
    </message>
    <message>
        <location filename="plugin/crmodbusplugin.cpp" line="233"/>
        <source>ms</source>
        <translation>мс</translation>
    </message>
    <message>
        <location filename="plugin/crmodbusplugin.cpp" line="245"/>
        <source>Retries</source>
        <translation>Повторы</translation>
    </message>
    <message>
        <location filename="plugin/crmodbusplugin.cpp" line="299"/>
        <source>Modbus/Request</source>
        <translation>Modbus/Запрос</translation>
    </message>
    <message>
        <location filename="plugin/crmodbusplugin.cpp" line="300"/>
        <source>Request</source>
        <translation>Запрос</translation>
    </message>
</context>
<context>
    <name>CrModbusRequest</name>
    <message>
        <location filename="crmodbusrequest.cpp" line="135"/>
        <source>Illegal function. Function code received in the query is not recognized or allowed by slave.</source>
        <oldsource>Illegal Function. Function code received in the query is not recognized or allowed by slave.</oldsource>
        <translation>Недопустимая функция. Код функции, отправленный в запросе, не распознается, или не разрешается ведомым устройством.</translation>
    </message>
    <message>
        <location filename="crmodbusrequest.cpp" line="139"/>
        <source>Illegal data address. Data address of some or all the required entities are not allowed or do not exist in slave.</source>
        <oldsource>Illegal Data Address. Data address of some or all the required entities are not allowed or do not exist in slave.</oldsource>
        <translation>Недопустимый адрес данных. Один или несколько адресов данных, отправленных в запросе, не разрешены или не существуют в подчиненном устройстве.</translation>
    </message>
    <message>
        <location filename="crmodbusrequest.cpp" line="143"/>
        <source>Illegal data value. Value is not accepted by slave.</source>
        <oldsource>Illegal Data Value. Value is not accepted by slave.</oldsource>
        <translation>Недопустимые данные. Данные, отправеленные в запросе, не принимаются подчиненным устройстовом.</translation>
    </message>
    <message>
        <location filename="crmodbusrequest.cpp" line="147"/>
        <source>Slave device failure. Unrecoverable error occurred while slave was attempting to perform requested action.</source>
        <oldsource>Slave Device Failure. Unrecoverable error occurred while slave was attempting to perform requested action.</oldsource>
        <translation>Ошибка ведомого устройства. Произошла неустранимая ошибка, когда ведомое устройство пыталось выполнить запрошенное действие.</translation>
    </message>
    <message>
        <location filename="crmodbusrequest.cpp" line="151"/>
        <source>Acknowledge. Slave has accepted request and is processing it, but a long duration of time is required.</source>
        <oldsource>Acknowledge.Slave has accepted request and is processing it, but a long duration of time is required.</oldsource>
        <translation>Запрос принят. Ведомое устройстов приняло запрос, но на его обработку требуется длительный период времени.</translation>
    </message>
    <message>
        <location filename="crmodbusrequest.cpp" line="155"/>
        <source>Slave device busy. Slave is engaged in processing a long-duration command. Master should retry later.</source>
        <oldsource>Slave Device Busy. Slave is engaged in processing a long-duration command. Master should retry later.</oldsource>
        <translation>Занято. Ведомое усторйстов занимается обработкой длительной команды. Мастер должен повторить запрос позже.</translation>
    </message>
    <message>
        <location filename="crmodbusrequest.cpp" line="159"/>
        <source>Negative acknowledge. Slave cannot perform the programming functions. Master should request diagnostic or error information from slave.</source>
        <oldsource>Negative Acknowledge. Slave cannot perform the programming functions. Master should request diagnostic or error information from slave.</oldsource>
        <translation>Запрос не принят. Ведомый не может выполнить программную функцию. Мастер должен запросить диагностическую информацию, или информацию об ошибках от подчиненного устройства.</translation>
    </message>
    <message>
        <location filename="crmodbusrequest.cpp" line="163"/>
        <source>Memory parity error. Slave detected a parity error in memory. Master can retry the request, but service may be required on the slave device.</source>
        <oldsource>Memory Parity Error. Slave detected a parity error in memory. Master can retry the request, but service may be required on the slave device.</oldsource>
        <translation>Ошибка четности памяти. Подчиненное устройство обнаружило ошибку четности в памяти. Мастер может повторить запрос, но возможно, подчиненное устойство неисправно.</translation>
    </message>
    <message>
        <location filename="crmodbusrequest.cpp" line="167"/>
        <source>Gateway path unavailable. Gateway misconfigured.</source>
        <oldsource>Gateway Path Unavailable. Specialized for Modbus gateways. Indicates a misconfigured gateway.</oldsource>
        <translation>Путь к шлюзу недоступен. Путь к шлюзу настроен неверно.</translation>
    </message>
    <message>
        <location filename="crmodbusrequest.cpp" line="171"/>
        <source>Gateway target device failed to respond. Slave fails to respond.</source>
        <oldsource>Gateway Target Device Failed to Respond. Specialized for Modbus gateways. Sent when slave fails to respond.</oldsource>
        <translation>Целевое устройство шлюза не ответило. Подчиненное устройстово не отвечает.</translation>
    </message>
    <message>
        <location filename="crmodbusrequest.cpp" line="175"/>
        <source>Timeout error.</source>
        <oldsource>Timeout error</oldsource>
        <translation>Ошибка по таймауту.</translation>
    </message>
    <message>
        <location filename="crmodbusrequest.cpp" line="178"/>
        <source>Fault сheck CRC code.</source>
        <oldsource>Check CRC code fault.</oldsource>
        <translation>Неудача при проверке контройльной суммы CRC.</translation>
    </message>
    <message>
        <location filename="crmodbusrequest.cpp" line="181"/>
        <source>Unknown error.</source>
        <translation>Неизвестная ошибка.</translation>
    </message>
</context>
</TS>
