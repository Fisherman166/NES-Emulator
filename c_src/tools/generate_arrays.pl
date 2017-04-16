#!/usr/bin/perl

use warnings;
use strict;
use Data::Dumper;

my %opcode_hash = &create_opcode_hash("opcodes.txt");
my @address_array = &generate_addressing_array(\%opcode_hash);
my @function_array = &generate_function_array(\%opcode_hash);
my @instruction_text = &generate_instruction_text_array(\%opcode_hash);
&write_arrays(\@address_array, \@function_array, \@instruction_text);
exit(0);

###############################################################################
## Functions
###############################################################################
sub create_opcode_hash() {
    my $source_filename = shift;
    my %opcode_hash;

    open(my $filehandle, "<", $source_filename) or die "Cannot open file $source_filename\n";
    while(my $line = <$filehandle>) {
        chomp $line;
        my @csv_split = split(/,/, $line);
        &extract_info(\%opcode_hash, @csv_split);
    }
    close($filehandle);
    return %opcode_hash;
}

sub extract_info() {
    my ($opcode_hash_ref, @split_opcode_line) = @_;

    if(exists $opcode_hash_ref->{$split_opcode_line[0]}) {
        print "ERROR: opcode $split_opcode_line[0] already exists\n";
        print "It currently points to $opcode_hash_ref->{$split_opcode_line[0]}\n";
        exit(-1);
    }

    if($split_opcode_line[1] =~ /(\w+)_(\w+)/) {
        $opcode_hash_ref->{$split_opcode_line[0]}->{'addressing_mode'} = $1;
        $opcode_hash_ref->{$split_opcode_line[0]}->{'instruction_text'} = $2;
        $opcode_hash_ref->{$split_opcode_line[0]}->{'instruction'} = $split_opcode_line[1];
    }
    else {
        print "$split_opcode_line[1] does not match regex\n";
        exit(-1);
    }
}

sub generate_addressing_array() {
    my ($opcode_hash_ref) = @_;
    my %opcode_hash = %{$opcode_hash_ref};
    my @addressing_array = ("ERR")x256;

    foreach my $opcode (keys %opcode_hash) {
        my $opcode_numerical = hex($opcode);
        my $opcode_text = &decode_addressing_mode($opcode_hash{$opcode}->{'addressing_mode'});
        $addressing_array[$opcode_numerical] = $opcode_text;
    }
    return @addressing_array;
}

sub decode_addressing_mode() {
    my $addressing_text = shift;

    if($addressing_text eq "implied") {return "IMP";}
    elsif($addressing_text eq "immediate") {return "IMM";}
    elsif($addressing_text eq "accumulator") {return "ACC";}
    elsif($addressing_text eq "zeropage") {return "ZRP";}
    elsif($addressing_text eq "zeropageX") {return "ZPX";}
    elsif($addressing_text eq "zeropageY") {return "ZPY";}
    elsif($addressing_text eq "absolute") {return "ABS";}
    elsif($addressing_text eq "absoluteX") {return "ABX";}
    elsif($addressing_text eq "absoluteY") {return "ABY";}
    elsif($addressing_text eq "indirectX") {return "INX";}
    elsif($addressing_text eq "indirectY") {return "INY";}
    elsif($addressing_text eq "relative") {return "REL";}
    elsif($addressing_text eq "indirect") {return "IND";}
    else {return "ERR";}
}

sub generate_function_array() {
    my ($opcode_hash_ref) = @_;
    my %opcode_hash = %{$opcode_hash_ref};
    my @function_array = ("NULL")x256;

    foreach my $opcode (keys %opcode_hash) {
        my $opcode_numerical = hex($opcode);
        $function_array[$opcode_numerical] = $opcode_hash{$opcode}->{'instruction'};
    }
    return @function_array;
}

sub generate_instruction_text_array() {
    my ($opcode_hash_ref) = @_;
    my %opcode_hash = %{$opcode_hash_ref};
    my @instruction_text = ("ERR")x256;

    foreach my $opcode (keys %opcode_hash) {
        my $opcode_numerical = hex($opcode);
        $instruction_text[$opcode_numerical] = $opcode_hash{$opcode}->{'instruction_text'};
    }
    return @instruction_text;
}

sub write_arrays() {
    my ($addressing_mode_ref, $functions_ref, $text_ref) = @_;
    my @addressing_modes = @{$addressing_mode_ref};
    my @functions = @{$functions_ref};
    my @function_text = @{$text_ref};

    my $target_filename = "arrays_out.txt";
    open(my $filehandle, ">", $target_filename) or die "Cannot open file $target_filename\n";

    print $filehandle "static const char* instruction_text[] = {\n";
    &write_array($filehandle, 1, @function_text);
    print $filehandle "};\n";

    print $filehandle "static const uint8_t instruction_addressing_mode[] = {\n";
    &write_array($filehandle, 0, @addressing_modes);
    print $filehandle "};\n";

    print $filehandle "static uint8_t (*instructions[]) (cpu_registers*) = {\n";
    &write_array($filehandle, 2, @functions);
    print $filehandle "};\n";

    close($filehandle);
}

sub write_array() {
    my ($filehandle, $write_type, @array_to_write) = @_;
    my $counter = 0;

    foreach my $element (@array_to_write) {
        chomp $element;
        if($write_type == 0) {
            print $filehandle "$element, ";
        }
        elsif($write_type == 1) {
            print $filehandle "\"$element\", ";
        }
        else {
            if($element eq "NULL") {
                print $filehandle "$element, ";
            }
            else {
                print $filehandle "&$element, ";
            }
        }
        $counter++;
        print $filehandle "\n" if(not ($counter % 16));
    }
}

