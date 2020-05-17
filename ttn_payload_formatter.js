function Decoder(bytes, port) {
    var decoded = {};

    var voltage = (bytes[0] << 8) + bytes[1] +(bytes[2] / 10.0);
    var power = (bytes[3] << 8) + bytes[4];
    var energy = (bytes[5] << 16) + (bytes[6] << 8) + bytes[7];


    decoded.voltage = voltage;
    decoded.current = power / voltage ;
    decoded.power = power;
    decoded.energy = energy;
    
    return decoded;
}